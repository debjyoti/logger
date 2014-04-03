This library provides functions to write log messages to files(and to
screen). It has the following benefits compared to directly doing
fprintf:

1. A separate thread writes the log messages. I/O time will be saved
if you are using a multi-core processor.

2. You can specify levels for the message like INFO, WARNING, ERROR.

3. Debug messages are removed by the preprocessor if _LOG_DEBUG flag
is not used during compilation.

4. INFO and debugging (if enabled) messages are buffered in-memory
before writing to the log file. 

5. The time (Hour:Min:Sec) and Level of a message will automatically
get prefixed to the message.

6. Easy log file rotation using log_file_change function.

<hr>
####Comparison with existing logging libraries (log4c and zlog):

Advantages of logger:

1. It uses a separate thread to log messages.

2. It uses in-memory buffering.

3. Simplicity. Only 6 functions and ~128 lines of code. 

Disadvantages:

1. Less features:
    a. Does not support flexible formatting.
    b. Does not support duplicate logging to muptiple files.
    c. Does not support auto-file rotation based on size/time.
    d. Does not support logging hierarchy.

2. Not thread safe. The libary uses global variables which are shared
between the main thread and logging thread. Hence, only 1 thread must
initialize the library and exit the library. Once the initialization
is done, the logging functions can be invoked by other threads in a
thread-safe way.

<hr>
####Reference
This concept of buffering and threading is inspired by
https://github.com/zma/zlog. Though the implementation is completely
different.

<hr>
####Using the interface
Using the library is simple; have a look at the test codes in the
tests/ folder. The library provides the following interface functions:

- log_init (Opens the file and creates the logger thread):
```c
int log_init(char* filename);
```

returns 0 for success. 

- log_enable_trace (Enables tracing, i.e. all messages are printed to screen)
```c
void log_enable_trace();
```

Not recommended for good performance, because buffering is disabled
when traceing is used.
   
- log_print (Log messages):
```c
void log_print(log_level lvl, char const* fmt, ...);
e.g.:
log_print(INFO, "Print string %s and integer %d", str, i);
log_print(WARNING, "Print string %s and integer %d", str, i);
log_print(ERROR, "Print string %s and integer %d", str, i);
```

- log_print_debug (Log debugging messages): 
```c
void log_print_debug(char const* fmt, ...)
e.g.: 
log_print_debug("Print string %s and integer %d", str, i);
```

Note: Your code must be compiled with `_LOG_DEBUG` flag for debugging
messages to be logged. Else they will be removed by the preprocessor.

- log_file_change (Rotate log file) :
```c
void log_file_change(char* new_filename);
e.g: 
log_file_change("/logpath/logfile2");
```

- log_exit (Cleans up the logger thread):
```c
void log_exit();
```

<hr>
####TESTSUITE
Execute the test/run_tests.sh file to execute all the tests in the
testsuite.

<hr>
**IMPORTANT:**
Only the log_print and log_debug_print functions are thread-safe.

<hr>
####AUTHOR
debjyoti.majumder@gmail.com
