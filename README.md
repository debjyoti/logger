C logging code using a different thread. 

Similar to https://github.com/zma/zlog, except the following:
 - Instead of looping continuously, it uses pthread_cond_wait.

I will not write much here, because the code is small and
self-explanatory if you understand pthreads.

Using the code is simple, have a look at the test codes in the tests/
folder.

*IMPORTANT*
Only the log_print and log_debug_print is thread-safe. All the other
functions must be invoked only from the parent thread if your code is
multithreaded.


Email: debjyoti.majumder@gmail.com
