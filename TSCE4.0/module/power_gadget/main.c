/*************************************************************************
	> File Name: main.c
	> Author: inspur
	> Mail: name@inspur.com 
	> Created Time: Fri 23 Oct 2015 09:47:31 AM CST
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
	while(1)
	{
		sleep(2);
		rapl_start();
	}
	return 0;
}
