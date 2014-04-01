#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>
#include "logger.h"

/* TODO: Test and remove unnecessary includes */

#define LOG_BUFFER_SIZE 1024
#define LOG_BUFFER_FLUSH_SIZE (0.8 * LOG_BUFFER_SIZE)

#define handle_error_en(en, msg) \
    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

typedef enum enum_sig{NO_SIGNAL, FLUSH_BUFFER, TERMINATE_THREAD}
enum_signal;
typedef enum enum_boolean{ NO = 0, YES = 1 } enum_bool;

/* global variables  */
pthread_t           g_logger_thread_id;
pthread_cond_t      g_log_buffer_cond  =PTHREAD_COND_INITIALIZER;
pthread_mutex_t     g_log_buffer_mutex =PTHREAD_MUTEX_INITIALIZER;
char                g_log_buffer[LOG_BUFFER_SIZE]\
                        [LOG_BUFFER_STR_MAX_LEN];
int                 g_log_buffer_size = 0;
enum_signal         g_log_buffer_cond_signal = NO_SIGNAL;
enum_bool           g_trace_on = NO;
enum_bool           g_initialized = NO;

static int _get_current_time(char* str, int max_len)
{
    struct timeval *epoch_time;
    struct tm *local_time;
    int time_str_len;
    epoch_time = (struct timeval *)malloc(sizeof(struct timeval));
    gettimeofday(epoch_time, NULL);
    local_time = localtime(&(epoch_time->tv_sec));
    time_str_len= strftime(str, max_len, "%H:%M:%S - ", local_time);
    return time_str_len;
}

/* Logger thread will be executing this function */
void* _logger_thread(void * filepath)
{
    FILE *fp_log;
    int i;
    char time_str[10];
    fp_log = fopen((char*)filepath, "a+");
    if (fp_log==NULL)
    {
        printf("Failed to open log file %s in append mode\n",
                filepath);
        exit(1);
    }
    _get_current_time(time_str, 10);
    printf("%sLog file %s created.\n", time_str, (char*)filepath);
    while(1)
    {
        pthread_mutex_lock(&g_log_buffer_mutex);

        /*COND_WAIT will unlock mutex and wait for signal.
          when it will receive signal, it will lock the 
          mutex and continue. But sometimes there can be 
          false signal. Hence, the cond_var while loop is 
          used. */
        while (g_log_buffer_cond_signal == NO_SIGNAL) { 
            pthread_cond_wait(&g_log_buffer_cond,
                    &g_log_buffer_mutex);
        }

        for(i = 0; i<g_log_buffer_size; i++)
        {
            fprintf(fp_log, "%s", g_log_buffer[i]);
        }
        fflush(fp_log);
        g_log_buffer_size = 0;

        if(g_log_buffer_cond_signal == TERMINATE_THREAD)
        {
            _get_current_time(time_str, 10);
            printf("%sClosing log file %s\n",time_str,(char*)filepath);
            fclose(fp_log);
            g_log_buffer_cond_signal = NO_SIGNAL;
            pthread_mutex_unlock(&g_log_buffer_mutex);
            return (void*)NULL;
        }
        g_log_buffer_cond_signal = NO_SIGNAL;
        pthread_mutex_unlock(&g_log_buffer_mutex);
    }
    fclose(fp_log);
    return (void*)NULL;
}

static void _create_thread(char* filename)
{
    int s;
    s = pthread_create(&g_logger_thread_id, NULL,
            &_logger_thread,(void *) filename);
    if(s != 0)
        handle_error_en(s, "pthread_create");
}

int log_enable_trace()
{
    g_trace_on = YES;
}

int log_init(char* filename)
{
    _create_thread(filename);
    g_initialized = YES;
    return 0; 
}

static inline void _signal_logger_thread(enum_signal sig)
{
    pthread_mutex_lock(&g_log_buffer_mutex);
    g_log_buffer_cond_signal = sig;
    pthread_cond_signal(&g_log_buffer_cond);
    pthread_mutex_unlock(&g_log_buffer_mutex);
}

void log_print(log_level lvl, char const* fmt, ...)
{
    if(!g_initialized)
    {
        fprintf(stderr, "ERROR: Log_init was not invoked\n");
        exit(1);
    }
    char tmp_buffer[LOG_BUFFER_STR_MAX_LEN];
    int prefix_length;
    int print_to_screen = 0;
    int print_to_file   = 0;

    /* Add datetime to each log string */
    prefix_length=_get_current_time(tmp_buffer,LOG_BUFFER_STR_MAX_LEN);

    switch (lvl)
    {
        case INFO:
            break;
        case WARNING:
            strncat(tmp_buffer,"WARN - ", 7);
            print_to_screen = 1;
            print_to_file = 1;
            break;
        case ERROR:
            strncat(tmp_buffer, "ERROR - ", 8);
            print_to_screen = 1;
            print_to_file = 1;
            break;
        default:
            break;
    }

    /* Print variable parameters to va_buffer. The length of va_buffer
       should be MAX_LEN - prefix_length. But malloc and free would be
       expensive. So I am using MAX_LEN here, but using
       MAX_LEN-prefix_length while doing the strncat. So all good ;-D
       Could have used VLA. But VLA internally takes up extra space.
       So, why bother so much about 10 chars? */
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
    pthread_mutex_unlock(&g_log_buffer_mutex);

    if(g_trace_on || print_to_screen)
    {
        printf(tmp_buffer);
    }
    if(print_to_file || (g_log_buffer_size > LOG_BUFFER_FLUSH_SIZE))
    {
        _signal_logger_thread(FLUSH_BUFFER);
    }
}

void log_exit()
{
    _signal_logger_thread(TERMINATE_THREAD);
    pthread_join(g_logger_thread_id, NULL);
}

void log_file_change(char* new_filename)
{
    log_exit();
    _create_thread(new_filename);
}
