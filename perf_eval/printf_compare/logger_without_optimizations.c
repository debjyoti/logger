#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <sched.h>
#include "logger_without_optimizations.h"

#define _MAX_TIME_STR_LEN 15

typedef enum enum_boolean{ NO = 0, YES = 1 } enum_bool;
enum_bool g_trace_on = NO;
FILE *fp_log;

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

void log_enable_trace()
{
    g_trace_on = YES;
}

int log_init(char* filename)
{
    fp_log = fopen((char*)filename, "a");
    if (fp_log==NULL)
    {
        printf("Failed to open log file '%s' in append mode\n",
                filename);
        exit(EXIT_FAILURE);
    }
    else
        _print_to_screen("Log file %s opened", filename);
    return 0; 
}

void log_print(log_level lvl, char const* fmt, ...)
{
    char tmp_buffer[LOG_BUFFER_STR_MAX_LEN];
    int prefix_length;
    int print_to_err = 0;

    /* Add datetime to each log string */
    prefix_length = _get_current_time( tmp_buffer,
            LOG_BUFFER_STR_MAX_LEN);

    switch (lvl)
    {
        case WARNING:
            strncat(tmp_buffer,"WARN - ", 7);
            print_to_err = 1;
            break;
        case ERROR:
            strncat(tmp_buffer, "ERROR - ", 8);
            print_to_err = 1;
            break;
        default:
            break;
    }

    char va_buffer[LOG_BUFFER_STR_MAX_LEN];
    va_list va;
    va_start(va, fmt);
    vsnprintf(va_buffer, LOG_BUFFER_STR_MAX_LEN, fmt, va);
    va_end(va);

    strncat(tmp_buffer, va_buffer,
            LOG_BUFFER_STR_MAX_LEN-prefix_length);

    if(print_to_err)
    {
        fprintf(stderr,"\n%s\n",tmp_buffer);
    }
    fprintf(fp_log, "%s", tmp_buffer);
    if(g_trace_on)
        printf("%s", tmp_buffer);
}

void log_exit()
{
    fclose(fp_log);
    _print_to_screen("Closed log file\n");
}

void log_file_change(char* new_filename)
{
    log_exit();
    log_init(new_filename);
}
