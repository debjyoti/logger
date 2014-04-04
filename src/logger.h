#ifndef _LOGGER_H
#define _LOGGER_H

#include <stdarg.h> /* for variable arguments used in debug func */

#define LOG_BUFFER_SIZE 1024
#define LOG_BUFFER_STR_MAX_LEN 128

typedef enum enum_log_level { ERROR, WARNING, INFO } log_level;

/* Function Definitions */

int log_init(char* filename);

void log_enable_trace();

void log_print(log_level lvl, char const* fmt, ...);

void log_exit();

void log_file_change(char* new_filename);

static inline void log_print_debug(char const* fmt, ...)
{
#ifdef _LOG_DEBUG
    char tmp_buffer[LOG_BUFFER_STR_MAX_LEN];
    va_list va;
    va_start(va, fmt);
    vsnprintf(tmp_buffer, LOG_BUFFER_STR_MAX_LEN, fmt, va);
    va_end(va);
    log_print(INFO, tmp_buffer);
#endif
}

#endif
