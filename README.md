C logging code using a different thread. 

Similar to https://github.com/zma/zlog, except the following:
 - Instead of looping continuously, it uses pthread_cond_wait.

Email: debjyoti.majumder@gmail.com
