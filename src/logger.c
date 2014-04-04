#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include "logger.h"

#define _MAX_TIME_STR_LEN 15

typedef enum enum_sig{NO_SIGNAL, FLUSH_BUFFER, TERMINATE_THREAD}
enum_signal;
typedef enum enum_boolean{ NO = 0, YES = 1 } enum_bool;

/*
 * The following global variables that are used by both threads. Hence
 * mutex locks should be used for both read & write
 */
pthread_cond_t      g_log_buffer_cond  =PTHREAD_COND_INITIALIZER;
pthread_mutex_t     g_log_buffer_mutex =PTHREAD_MUTEX_INITIALIZER;
char                g_log_buffer[LOG_BUFFER_SIZE]\
                        [LOG_BUFFER_STR_MAX_LEN];
int                 g_log_buffer_size = 0;
enum_signal         g_log_buffer_cond_signal = NO_SIGNAL;
enum_bool           g_initialized = NO;
enum_bool           g_trace_on = NO;

/* global variables only used by main thread */
pthread_t           g_logger_thread_id;
int                 g_log_buffer_flush_size = 0.8 * LOG_BUFFER_SIZE;

static void _handle_error_en(int en, char* msg)
{
    errno = en;
    perror(msg);
    exit(EXIT_FAILURE);
}

static int _get_current_time(char* str, int max_len)
{
    struct timeval *epoch_time;
    struct tm *local_time;
    int time_str_len;
    epoch_time = (struct timeval *)malloc(sizeof(struct timeval));
    gettimeofday(epoch_time, NULL);
    local_time = localtime(&(epoch_time->tv_sec));
    time_str_len= strftime(str, max_len, "%H:%M:%S - ", local_time);
    if(!time_str_len) /* 0 when max_len is shorter than format */
    {
        time_str_len = max_len-1;
        str[max_len-1]= '\0';
    }
    return time_str_len;
}

static void _print_to_screen(char const* fmt, ...)
{
    char time_str[_MAX_TIME_STR_LEN];
    char msg_str[LOG_BUFFER_STR_MAX_LEN];
    _get_current_time(time_str, _MAX_TIME_STR_LEN);
    va_list va;
    va_start(va, fmt);
    vsnprintf(msg_str, LOG_BUFFER_STR_MAX_LEN, fmt, va);
    va_end(va);
    printf("\n%s%s\n", time_str, msg_str);
    fflush(stdout);
}

/* Logger thread will be executing this function 
 * CAUTION: Any function invoked from within _logger_thread must be
 * thread-safe */
void* _logger_thread(void * filepath)
{
    FILE *fp_log;
    int i;
    int *exit_status;
    exit_status = (int *)malloc(sizeof(int));
    *exit_status = 0;

    fp_log = fopen((char*)filepath, "a");
    if (fp_log==NULL)
    {
        printf("Failed to open log file '%s' in append mode\n",
                filepath);
        exit(EXIT_FAILURE);
    }
    pthread_mutex_lock(&g_log_buffer_mutex);
    /* make sure that logger_thread reach here before signals are
     * sent*/
    g_initialized = YES; 
    while(1)
    {
        /*COND_WAIT will unlock mutex and wait for signal.
          when it will receive signal, it will lock the 
          mutex and continue. But sometimes there can be 
          false signal. Hence, the cond_var while loop is 
          used. */
        while (g_log_buffer_cond_signal == NO_SIGNAL) { 
            pthread_cond_wait(&g_log_buffer_cond,
                    &g_log_buffer_mutex);
        }

#ifdef _LOG_DEBUG
        printf("Logger Thread: About to print %d msgs\n",
                g_log_buffer_size);
#endif
        if(g_trace_on)
        {
            for(i = 0; i<g_log_buffer_size; i++)
            {
                fprintf(stdout, "%s", g_log_buffer[i]);
                fprintf(fp_log, "%s", g_log_buffer[i]);
            }
            fflush(stdout);
            fflush(fp_log);
        }
        else
        {
            for(i = 0; i<g_log_buffer_size; i++)
            {
                fprintf(fp_log, "%s", g_log_buffer[i]);
            }
            fflush(fp_log);
        }
        g_log_buffer_size = 0;

        if(g_log_buffer_cond_signal == TERMINATE_THREAD)
        {
            fclose(fp_log);
            g_log_buffer_cond_signal = NO_SIGNAL;
            *exit_status = 1;
            pthread_mutex_unlock(&g_log_buffer_mutex);
            pthread_exit((void*)exit_status);
        }
        g_log_buffer_cond_signal = NO_SIGNAL;
    }
    fclose(fp_log);
    return (void*)NULL;
}

void log_enable_trace()
{
    pthread_mutex_lock(&g_log_buffer_mutex);
    g_log_buffer_flush_size = TRACE_BUFFER_SIZE;
    g_trace_on = YES;
    pthread_mutex_unlock(&g_log_buffer_mutex);
}

int log_init(char* filename)
{
    int thread_not_yet_ready = 1;
    int rc;
    rc = pthread_create(&g_logger_thread_id, NULL,
            &_logger_thread,(void *) filename);
    if(rc != 0)
        _handle_error_en(rc, "pthread_create");
    while(thread_not_yet_ready)
    {
        pthread_mutex_lock(&g_log_buffer_mutex);
        if(g_initialized)
        {
            _print_to_screen("New logger thread created with id %d\n"
                    , g_logger_thread_id);
            _print_to_screen("Log file '%s' opened\n", filename);
            _print_to_screen(
                    "LogBuffer will be flushed after ~%d msg\n",
                    g_log_buffer_flush_size);
            thread_not_yet_ready = 0;
        }
        pthread_mutex_unlock(&g_log_buffer_mutex);
    }
    return 0; 
}

/* Note the mutex must be locked before calling this func */
static inline void _wait_for_logger_thread()
{
    while(g_log_buffer_cond_signal != NO_SIGNAL)
    {
        pthread_mutex_unlock(&g_log_buffer_mutex);
        if(g_logger_thread_id)
            sleep(1);
        else
            _handle_error_en(-4, "Logger thread is dead\n");
        pthread_mutex_lock(&g_log_buffer_mutex);
    }
}

static inline void _signal_logger_thread(enum_signal sig)
{
    pthread_mutex_lock(&g_log_buffer_mutex);
    /* Race Condition: Logger thread might not have processed the
     * previous signal yet */
    if(g_log_buffer_cond_signal == sig)
    {
        /* Last signal is not yet processed. Since it was the same
         * signal no need to send this one */
        pthread_mutex_unlock(&g_log_buffer_mutex);
        return;
    }
    else if (g_log_buffer_cond_signal != NO_SIGNAL)
    {
        /* Last signal is not yet processed. It was a different
         * signal. Need to wait before sending the new signal. */ 
        _wait_for_logger_thread();
    }
#ifdef _LOG_DEBUG
    _print_to_screen(
            "Main Thread: Sending signal to print %d msg\n",
            g_log_buffer_size);
#endif
    g_log_buffer_cond_signal = sig;
    pthread_cond_signal(&g_log_buffer_cond);
    pthread_mutex_unlock(&g_log_buffer_mutex);
}

void log_print(log_level lvl, char const* fmt, ...)
{
    if(!g_logger_thread_id)
    {
        _handle_error_en(-3, "Fatal Error: Logger thread id is 0.\
        Did you forget to invoke log_init?\n");
    }
    char tmp_buffer[LOG_BUFFER_STR_MAX_LEN];
    int prefix_length;
    int print_to_err = 0;
    int print_to_screen = 0;
    int print_to_file   = 0;

    /* Add datetime to each log string */
    prefix_length=_get_current_time(tmp_buffer,LOG_BUFFER_STR_MAX_LEN);

    switch (lvl)
    {
        case WARNING:
            strncat(tmp_buffer,"WARN - ", 7);
            print_to_err = 1;
            print_to_file = 1;
            break;
        case ERROR:
            strncat(tmp_buffer, "ERROR - ", 8);
            print_to_err = 1;
            print_to_file = 1;
            break;
        default:
            break;
    }

    /* Print variable parameters to va_buffer. The length of
     * va_buffer should be MAX_LEN - prefix_length. But malloc and
     * free would be expensive. So I am using MAX_LEN here, but
     * using MAX_LEN-prefix_length while doing the strncat. So all
     * good ;-D Could have used VLA. But VLA internally takes up
     * extra space. So, why bother so much about 10 chars? */
    char va_buffer[LOG_BUFFER_STR_MAX_LEN];
    va_list va;
    va_start(va, fmt);
    vsnprintf(va_buffer, LOG_BUFFER_STR_MAX_LEN, fmt, va);
    va_end(va);

    strncat(tmp_buffer, va_buffer,
            LOG_BUFFER_STR_MAX_LEN-prefix_length);

    pthread_mutex_lock(&g_log_buffer_mutex);
	strncpy(g_log_buffer[g_log_buffer_size], tmp_buffer,
			LOG_BUFFER_STR_MAX_LEN);
    g_log_buffer_size++;
    if(g_log_buffer_size == g_log_buffer_flush_size)
        print_to_file = 1;
    else if(g_log_buffer_size >= LOG_BUFFER_SIZE)
        _wait_for_logger_thread();
    pthread_mutex_unlock(&g_log_buffer_mutex);

    if(print_to_err)
    {
        fprintf(stderr,"\n%s\n",tmp_buffer);
    }
    if(print_to_file)
    {
        _signal_logger_thread(FLUSH_BUFFER);
    }
}

void log_exit()
{
    int *thread_return_status;
    int rc = 1;
    _signal_logger_thread(TERMINATE_THREAD);
    rc = pthread_join(g_logger_thread_id,
            (void **)&thread_return_status);
    if(0==rc && thread_return_status && 1 == (*thread_return_status) )
    {
        _print_to_screen("Closed log file\n");
        _print_to_screen("Logger thread terminated successfully\n");
        free(thread_return_status); 
    }
    else if(rc)
        _handle_error_en(rc, "pthread_join");
    else
        _handle_error_en(-2,
                "pthread_exit did not return proper exit status");
}

void log_file_change(char* new_filename)
{
    log_exit();
    log_init(new_filename);
}
