/*************************************************************************
	> File Name: time.c
	> Author: inspur
	> Mail: name@inspur.com 
	> Created Time: Fri 23 Oct 2015 02:30:32 PM CST
 ************************************************************************/

#include<stdio.h>
#include <sys/time.h>
#include <time.h>



double
convert_time_to_sec(struct timeval tv)
{
    double elapsed_time = (double)(tv.tv_sec) + ((double)(tv.tv_usec)/1000000);
    return elapsed_time;
}



int main()
{
	double end, start;
    struct timeval tv;

	while(1)
	{
		gettimeofday(&tv, NULL);
		 start = convert_time_to_sec(tv);
    
		printf("time is = %.4lf\n",start);
	}
}
