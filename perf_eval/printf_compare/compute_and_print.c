#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#ifdef DISABLE_OPTIMIZATIONS
#include "logger_without_optimizations.h"
#else
#include "../../src/logger.h"
#endif

#define PI 3.14
#define MATRIX_SIZE 100

double do_some_computation(double *d1, double *degree)
{
    double a[MATRIX_SIZE][MATRIX_SIZE], b[MATRIX_SIZE][MATRIX_SIZE], c[MATRIX_SIZE][MATRIX_SIZE];
    double sum = 0;
    int i, j, k;
    double ret = 0;
    for(i=0;i<MATRIX_SIZE;i++)
        for(j=0;j<MATRIX_SIZE;j++)
        {
            *d1 = sin((*degree) * (PI/180)); /* convert degree to radian */
            if(*d1<0.01) *d1=0.5;
            else if(*d1>0.98) *d1=0.1;
            *degree /= *d1;
            if(*degree > 10000000) *degree = 30.0;
            a[i][j]=*degree;
            b[i][j]=*d1;
        }
    for(i=0;i<MATRIX_SIZE;i++)
        for(j=0;j<MATRIX_SIZE;j++)
        {
            sum = 0;
            for(k=0;k<MATRIX_SIZE;k++)
                sum = sum + a[i][k] * b[k][j];
            c[i][j]=sum;
        }
    for(i=0;i<MATRIX_SIZE;i++)
        for(j=0;j<MATRIX_SIZE;j++)
           ret+=c[i][j]; 
    return ret;
}

int main( int argc, char* argv[])
{
    int msg_count;
    double degree=30.0;
    double d1;
    double ret_val;
    clock_t cpu_ticks;
    struct timeval starttime, endtime;
    double elapsed_clocktime;
    int loop_till;
    gettimeofday(&starttime, NULL);

    if(argc > 1)
        loop_till = atoi(argv[1]);
    else 
        loop_till = 4096; 

    printf("Executing %d computations and %d prints\n", loop_till,
            loop_till*2);

#ifdef DISABLE_OPTIMIZATIONS
    log_init("../out/compute_and_print_no_opt.log");
#else
    log_init("../out/compute_and_print.log");
#endif
    for( msg_count=1; msg_count<=loop_till; msg_count++)
    {
        log_print(INFO, "sin_val = %lf, degree = %lf\n", d1, degree);
        ret_val = do_some_computation(&d1, &degree);
        log_print(INFO, "Ret sin_val = %lf\n", ret_val);
    }
    log_exit();

    cpu_ticks = clock();
    gettimeofday(&endtime, NULL);
    elapsed_clocktime = (endtime.tv_sec-starttime.tv_sec)+
        (double)(endtime.tv_usec-starttime.tv_usec)/1000000;

    printf("cpu_ticks = %d, ticks per second = %d, cpu-seconds = %lf,\
    elapsed clocktime = %lf \n", (int)cpu_ticks,
            CLOCKS_PER_SEC, (double)cpu_ticks/CLOCKS_PER_SEC,
            elapsed_clocktime);
    return 0;
}
