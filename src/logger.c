#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <sched.h>
#include "logger.h"

#define _MAX_TIME_STR_LEN 15
#define _MAX_YIELD_WAIT 2000

typedef enum enum_sig{NO_SIGNAL, FLUSH_BUFFER, TERMINATE_THREAD}
enum_signal;
typedef enum enum_boolean{ NO = 0, YES = 1 } enum_bool;

/*
 * The following global variables that are used by both threads.
 * Hence mutex locks should be used for both read & write
 */
pthread_cond_t      g_log_cond  =PTHREAD_COND_INITIALIZER;
pthread_mutex_t     g_log_mutex =PTHREAD_MUTEX_INITIALIZER;
char                g_log_buffer[LOG_BUFFER_SIZE]\
                        [LOG_BUFFER_STR_MAX_LEN];
int                 g_log_buffer_fill = 0;
enum_signal         g_signal = NO_SIGNAL;
enum_bool           g_initialized = NO;
enum_bool           g_trace_on = NO;

/* global variables only used by main thread */
pthread_t           g_logger_thread_id;
int                 g_log_buffer_flush_size = 0.8*LOG_BUFFER_SIZE;

static void _handle_error_en(int en, char* msg)
{
    errno = en;
    perror(msg);
    exit(EXIT_FAILURE);
}

static int _get_current_time(char* str, int max_len)
{
    struct timeval epoch_time;
    struct tm *local_time;
    int time_str_len;
    gettimeofday(&epoch_time, NULL);
    local_time = localtime(&(epoch_time.tv_sec));
    time_str_len=strftime(str, max_len, "%H:%M:%S - ", local_time);
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

/* Logger thread will be executing this function.
 * CAUTION: Any function invoked from within _logger_thread must be
 * thread-safe.  Currently only printf, malloc, fopen, fclose and
 * fflush are used, which are thread safe (though not reentrant).
 */
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
    pthread_mutex_lock(&g_log_mutex);
    /* make sure that logger_thread reach here before signals are
     * sent. Otherwise the signal will be lost. */
    g_initialized = YES; 
    while(1)
    {
        /*COND_WAIT will unlock mutex and wait for signal.
          when it will receive signal, it will lock the 
          mutex and continue. But sometimes there can be 
          false signal. Hence, the g_signal while loop is
          used. */
        while (g_signal == NO_SIGNAL) { 
            pthread_cond_wait(&g_log_cond,
                    &g_log_mutex);
        }

#ifdef _LOG_DEBUG
        printf("Logger Thread: About to print %d msgs\n",
                g_log_buffer_fill);
#endif
        if(g_trace_on)
        {
            for(i = 0; i<g_log_buffer_fill; i++)
            {
                fprintf(stdout, "\n%s\n", g_log_buffer[i]);
                fprintf(fp_log, "%s", g_log_buffer[i]);
            }
            fflush(NULL);
        }
        else
        {
            for(i = 0; i<g_log_buffer_fill; i++)
            {
                fprintf(fp_log, "%s", g_log_buffer[i]);
            }
        }
        g_log_buffer_fill = 0;

        if(g_signal == TERMINATE_THREAD)
        {
            fclose(fp_log);
            g_signal = NO_SIGNAL;
            *exit_status = 1;
            pthread_mutex_unlock(&g_log_mutex);
            pthread_exit((void*)exit_status);
        }
        g_signal = NO_SIGNAL;
    }
    /* control will never reach here. Anyway, let me put these */
    fclose(fp_log);
    pthread_mutex_unlock(&g_log_mutex);
    return (void*)NULL;
}

void log_enable_trace()
{
    pthread_mutex_lock(&g_log_mutex);
    g_log_buffer_flush_size = 1; /* no buffering */
    g_trace_on = YES;
    pthread_mutex_unlock(&g_log_mutex);
}

int log_init(char* filename)
{
    enum_bool thread_ready = NO;
    int rc;
    rc = pthread_create(&g_logger_thread_id, NULL,
            &_logger_thread,(void *) filename);
    if(rc != 0)
        _handle_error_en(rc, "pthread_create");
    while(!thread_ready)
    {
        pthread_mutex_lock(&g_log_mutex);
        if(g_initialized)
        {
            pthread_mutex_unlock(&g_log_mutex);
            _print_to_screen("New logger thread # %d created \n"
                    , g_logger_thread_id);
            _print_to_screen("Log file '%s' opened\n", filename);
            thread_ready = YES;
        }
        else
        {
            pthread_mutex_unlock(&g_log_mutex);
            sched_yield();
        }
    }
    return 0; 
}

static void _check_logger_thread_is_active()
{
    int rc;
    if(!g_logger_thread_id)
    {
        _handle_error_en(-3, "Fatal Error: Logger thread id is 0.\
        Did you forget to invoke log_init?\n");
    }
    /* pthread_kill is used here to check if the thread exists.
     * sig is 0, so no signal is sent, but error checking is
     * still performed */
    rc = pthread_kill(g_logger_thread_id,0);
    if( rc ) /* rc 0 means that the thread is active */
        _handle_error_en(rc , "Logger thread is dead\n");
}

static int _wait_for_logger_thread()
{
    int rc, yield_count;
    _check_logger_thread_is_active();
    yield_count = 0;
    pthread_mutex_lock(&g_log_mutex);
#ifdef _LOG_DEBUG
    _print_to_screen("Main Thread: Waiting for logger thread.\
        Buffer has %d msgs.\n", g_log_buffer_fill);
#endif
    while(g_signal != NO_SIGNAL)
    {
        pthread_mutex_unlock(&g_log_mutex);
        sched_yield();
        yield_count++;
        if(yield_count == _MAX_YIELD_WAIT)
            sleep(2);
        if(yield_count > _MAX_YIELD_WAIT)
        {
            _print_to_screen("WARNING: Logger thread is hung.\
            Some log messages will not be printed.\n");
            return -1;
        }
        pthread_mutex_lock(&g_log_mutex);
    }
#ifdef _LOG_DEBUG
    _print_to_screen("Main Thread: Logger done after %d yield.\
        Buffer has %d msgs.\n", yield_count, g_log_buffer_fill);
#endif
    pthread_mutex_unlock(&g_log_mutex);
    return 0;
}

static inline void _signal_logger_thread(enum_signal sig)
{
    pthread_mutex_lock(&g_log_mutex);

    /* Race Condition: Logger thread might not have processed the
     * previous signal yet */
    if(g_signal == sig)
    {
        /* Last signal is not yet processed. Since it was the same
         * signal no need to send this one */
        pthread_mutex_unlock(&g_log_mutex);
        return;
    }
    else if (g_signal != NO_SIGNAL)
    {
        /* Last signal is not yet processed. It was a different
         * signal. Need to wait before sending the new signal. */ 
#ifdef _LOG_DEBUG
        _print_to_screen("Curr sig=%s, New sig=%s. Gonna Wait",
                g_signal==FLUSH_BUFFER?"FLUSH_BUFFER":"TERMINATE",
                sig==FLUSH_BUFFER?"FLUSH_BUFFER":"TERMINATE");
#endif
        pthread_mutex_unlock(&g_log_mutex);
        _wait_for_logger_thread();
        pthread_mutex_lock(&g_log_mutex);
    }
#ifdef _LOG_DEBUG
    _print_to_screen(
            "Main Thread: Sending signal to print %d msg\n",
            g_log_buffer_fill);
#endif
    g_signal = sig;
    pthread_cond_signal(&g_log_cond);
    pthread_mutex_unlock(&g_log_mutex);
    if(g_trace_on)
        sched_yield();
}

void log_print(log_level lvl, char const* fmt, ...)
{
    char tmp_buffer[LOG_BUFFER_STR_MAX_LEN];
    int prefix_length;
    int print_to_err = 0;
    int print_to_file   = 0;

    /* Add datetime to each log string */
    prefix_length = _get_current_time( tmp_buffer,
            LOG_BUFFER_STR_MAX_LEN);

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

    /* Print variable parameters to tmp_buffer:
     * The length of va_buffer should be MAX_LEN - prefix_length.
     * But malloc and free would be expensive. So I am using
     * MAX_LEN here, but using MAX_LEN-prefix_length while doing
     * the strncat. So all good ;-D */
    char va_buffer[LOG_BUFFER_STR_MAX_LEN];
    va_list va;
    va_start(va, fmt);
    vsnprintf(va_buffer, LOG_BUFFER_STR_MAX_LEN, fmt, va);
    va_end(va);
    strncat(tmp_buffer, va_buffer,
            LOG_BUFFER_STR_MAX_LEN-prefix_length);

    /* Copy tmp_buffer to global log buffer */
    pthread_mutex_lock(&g_log_mutex);
	strncpy(g_log_buffer[g_log_buffer_fill], tmp_buffer,
			LOG_BUFFER_STR_MAX_LEN);
    g_log_buffer_fill++;
    if(g_log_buffer_fill == g_log_buffer_flush_size)
    {
        print_to_file = 1;
    }
    else if(g_log_buffer_fill >= LOG_BUFFER_SIZE)
    /* Check global log buffer overflow:
     * Why overflow?
     * Because, when a signal is sent to the logger thread, we do
     * not wait for the logger thread to finish processing. We
     * simply carry on putting more stuff in the log buffer, and
     * wait only when there is an overflow.*/
    {
#ifdef _LOG_DEBUG
        _print_to_screen("Buffer size overflow. Buffer=%d. Max=%d\
        Gonna Wait", g_log_buffer_fill, LOG_BUFFER_SIZE);
#endif
        pthread_mutex_unlock(&g_log_mutex);
        if (_wait_for_logger_thread() == -1)
        /* error in wait. will overwrite log buffer */
        {
            pthread_mutex_lock(&g_log_mutex);
            g_log_buffer_fill = 0;
            pthread_mutex_unlock(&g_log_mutex);
        }
        pthread_mutex_lock(&g_log_mutex);
    }
    pthread_mutex_unlock(&g_log_mutex);

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
    if(0==rc && thread_return_status &&
            1 == (*thread_return_status) )
    {
        _print_to_screen("Closed log file\n");
        _print_to_screen("Logger thread terminated\n");
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
