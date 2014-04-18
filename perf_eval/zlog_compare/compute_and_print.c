#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#ifdef DISABLE_ZLOG
#include "../logger_without_optimizations.h"
#else
#include "zlog.h"
zlog_category_t* mycat = NULL;
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
	{
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
#ifdef DISABLE_ZLOG
		log_print(INFO, "a[%d][%d] = %lf\n", i, j, a[i][j]);
		log_print(INFO, "b[%d][%d] = %lf\n", i, j, b[i][j]);
#else
		zlog_info(mycat, "a[%d][%d] = %lf", i, j, a[i][j]);
		zlog_info(mycat, "b[%d][%d] = %lf", i, j, b[i][j]);
#endif
	}
	for(i=0;i<MATRIX_SIZE;i++)
	{
		for(j=0;j<MATRIX_SIZE;j++)
		{
			sum = 0;
			for(k=0;k<MATRIX_SIZE;k++)
				sum = sum + a[i][k] * b[k][j];
			c[i][j]=sum;
		}
#ifdef DISABLE_ZLOG
		log_print(INFO, "sum = %lf\n", sum);
#else
		zlog_info(mycat, "sum = %lf", sum);
#endif
	}
	for(i=0;i<MATRIX_SIZE;i++)
	{
		for(j=0;j<MATRIX_SIZE;j++)
		{
			ret+=c[i][j]; 
		}
#ifdef DISABLE_ZLOG
		log_print(INFO, "i=%d, ret=%lf\n", i, ret);
#else
		zlog_info(mycat, "i=%d, ret=%lf", i, ret);
#endif
	}
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

	printf("Executing %d computations \n", loop_till,
			loop_till*2);

#ifdef DISABLE_ZLOG
	log_init("compute_and_print_no_opt.log");
#else
	zlog_init("compute_and_print.conf");
	mycat = zlog_get_category("compute_and_print_category");
#endif
	for( msg_count=1; msg_count<=loop_till; msg_count++)
	{
#ifdef DISABLE_ZLOG
		log_print(INFO, "sin_val = %lf\n", degree);
		log_print(INFO, "degree = %lf\n", degree);
#else
		zlog_info(mycat, "sin_val = %lf", degree);
		zlog_info(mycat, "degree = %lf", degree);
#endif
		ret_val = do_some_computation(&d1, &degree);
#ifdef DISABLE_ZLOG
		log_print(INFO, "Computation completed for iteration %d\n",
				msg_count);
		log_print(INFO, "Ret sin_val = %lf\n", ret_val);
#else
		zlog_info(mycat, "Computation completed for iteration %d", msg_count);
		zlog_info(mycat, "Ret sin_val = %lf", ret_val);
#endif
	}
#ifdef DISABLE_ZLOG
	log_exit();
#else
	zlog_fini();
#endif

	cpu_ticks = clock();
	gettimeofday(&endtime, NULL);
	elapsed_clocktime = (endtime.tv_sec-starttime.tv_sec)+
		(double)(endtime.tv_usec-starttime.tv_usec)/1000000;

	printf("cpu_ticks = %d, ticks per second = %d, cpu-seconds = %lf,\
			elapsed clocktime = %lf \n", (int)cpu_ticks,
			CLOCKS_PER_SEC, (double)cpu_ticks/CLOCKS_PER_SEC,
			elapsed_clocktime);
	fprintf(stderr, "cpu_ticks = %d, ticks per second = %d, cpu-seconds = %lf,\
			elapsed clocktime = %lf \n", (int)cpu_ticks,
			CLOCKS_PER_SEC, (double)cpu_ticks/CLOCKS_PER_SEC,
			elapsed_clocktime);
	return 0;
}
