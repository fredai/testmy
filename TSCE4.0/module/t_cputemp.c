/*
 * Copyright (C) Inspur(Beijing)
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "con_define.h"
#include "framework.h"
#include "common.h"

#define CPUTEMP_MODULE_COL_NUM 1

static struct mod_info cputemp_info[] = {
	{"cpu_temp","\0"},
};

#define CALC_INTERVAL 20
static unsigned count = 0;
static float temp = 0;


void 
cputemp_start()
{
	//NULL
}

void 
cputemp_read(struct module *mod)
{

	assert(mod != NULL);

	char buf[1024] = {0};
	FILE *fp;
	
	if (count++ % CALC_INTERVAL != 0)
	{
		snprintf((mod->info[0]).index_data, LEN_32, "%.1f", temp);
		return;
	}
	if ((fp = popen("sensors 2>/dev/null", "r")) == NULL) 
	{
		printf("failed to open sensors");
		return;
	}

	float sum = 0;
	float temp_sum = 0;
	float tmp = 0;
	unsigned n = 0;
	unsigned m = 0;
	char flag;
	
	while (fgets(buf,1024,fp)) 
	{
		if(strlen(buf) <= 0) continue; 		
		
		if (strncasecmp(buf, "temp", 4) == 0)
		{
			if (sscanf(buf, "%*[^+-]%c%f¡ãC%*s", &flag, &tmp) == 2)
			{
				if (flag == '+')
				{
					temp_sum += tmp;
				}
				else
				{
					temp_sum -= tmp;	
				}
				++m;
			}
		}			
	
		if (strncasecmp(buf, "Physical", 8) == 0)
		{
			//Physical id 0:  +0.0¡ãC  (high = +100.0¡ãC, crit = +100.0¡ãC) 
			if (sscanf(buf, "%*[^+-]%c%f¡ãC%*s", &flag, &tmp) == 2)
			{
				if (flag == '+')
				{
					sum += tmp;
				}
				else
				{
					sum -= tmp;
				}
				++n;
			}
		}
		memset(buf, 0, sizeof(buf));	
	}
	fclose(fp);
	assert (mod -> col == CPUTEMP_MODULE_COL_NUM);
	
	if ( n > 0)
	{
		temp = sum/n;
	}
	else
	{
		if (m == 0) m = 1;
		temp = temp_sum/m;
	}
	snprintf((mod->info[0]).index_data, LEN_32, "%.1f", temp);
}

int
mod_register(struct module *mod)
{
	assert(mod != NULL);

	FILE *fp;
	char error_buf[LEN_64];

	fp = popen("sensors 2>&1", "r");
        fgets(error_buf,LEN_64, fp);

        if (strstr(error_buf, "not found"))
        {
                return MODULE_FLAG_NOT_USEABLE;
        }
	fclose(fp);
	/* TODO: add decide module is useable in current HW and SW environment */

	register_module_fields(mod, cputemp_info, CPUTEMP_MODULE_COL_NUM, \
				 cputemp_start, cputemp_read);
	return 0;
}
