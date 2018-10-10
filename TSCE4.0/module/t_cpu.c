
/*
 * Copyright (C) Inspur(Bejing)
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>

#include "con_define.h"
#include "framework.h"
#include "common.h"

#define CPU_MODULE_COL_NUM 4

struct  t_cpu
{
	unsigned long long user;
	unsigned long long nice;
	unsigned long long sys;
	unsigned long long idle;
	unsigned long long iowait;
	unsigned long long hardirq;
	unsigned long long softirq;
	unsigned long long steal;
	/* unsigned long	   second; */
};


static struct t_cpu  cpu_s, cpu_e, cpu_t;

static struct mod_info cpu_mod_info [] = {
	{"cpu_user", "\0"},
	{"cpu_sys", "\0"},
	{"cpu_iowait", "\0"},
	{"cpu_idle",  "\0"}
};

#if 0
unsigned long get_current_second() 
{
	struct timeval tv = {0,0};

	if (-1 == gettimeofday(&tv, NULL)) {
		perror("gettimeofday");	
		return -1;
	}
	return tv.tv_sec;
}
#endif

void 
cpu_start()
{
	FILE *fd;
	char buf[1024];
	
#if 0
	if ((cpu_s.second = get_current_second()) < 0) {
		printf("%s:second = %d\n", __FUNCTION__, cpu_s.second);
		return;
	}
#endif

	fd = fopen ("/proc/stat", "r");
	if (NULL == fd) {
		perror("fopen /proc/stat");
		return;
	}
	if (0 != fgets(buf,1024,fd)) {

		sscanf(buf+5, "%lld %lld %lld %lld %lld %lld %lld %lld", 
				&cpu_s.user,
				&cpu_s.nice,
				&cpu_s.sys,
				&cpu_s.idle,
				&cpu_s.iowait,
				&cpu_s.hardirq,
				&cpu_s.softirq,
				&cpu_s.steal); 
	}
	fclose(fd);     

}


void 
cpu_read(struct module *mod)
{
	assert(mod != NULL);

	float cpu_user = 0.0;
	float cpu_sys = 0.0;
	float cpu_iowait = 0.0;
	float cpu_idle = 0.0;
	//float cpu_total = 0.0;
	unsigned long long cpu_total = 0;

	FILE *fd;
	char buf[1024];

#if 0
	if ((cpu_e.second = get_current_second()) < 0) {
		printf("%s:second = %d\n", __FUNCTION__, cpu_e.second);
		return;
	}
#endif

	fd = fopen("/proc/stat", "r");
	if (NULL == fd) {
		perror("fopen /proc/stat");
		return;
	}
	if (0 != fgets(buf, 1024, fd)) {
		sscanf(buf+5, "%lld %lld %lld %lld %lld %lld %lld %lld", 
				&cpu_e.user,
				&cpu_e.nice,
				&cpu_e.sys,
				&cpu_e.idle,
				&cpu_e.iowait,
				&cpu_e.hardirq,
				&cpu_e.softirq,
				&cpu_e.steal);
	} else {
		printf("fgets /proc/stat buf = 0\n");
		fclose(fd);
		return;
	}
#if 0
	if ((cpu_t.second = cpu_e.second - cpu_s.second) <= 0) {
    	printf("%s:cpu_e.second - cpu_s.sencod = %d\n", cpu_t.second);
		return;
	}
#endif

        cpu_t.user = (cpu_e.user>=cpu_s.user) ? (cpu_e.user-cpu_s.user) : cpu_e.user;
        cpu_t.nice = (cpu_e.nice>=cpu_s.nice) ? (cpu_e.nice-cpu_s.nice) : cpu_e.nice;
        cpu_t.sys = (cpu_e.sys>=cpu_s.sys) ? (cpu_e.sys-cpu_s.sys) : cpu_e.sys;
        cpu_t.idle = (cpu_e.idle>=cpu_s.idle) ? (cpu_e.idle-cpu_s.idle) : cpu_e.idle;
        cpu_t.iowait = (cpu_e.iowait>=cpu_s.iowait) ? (cpu_e.iowait-cpu_s.iowait) : cpu_e.iowait;
        cpu_t.hardirq = (cpu_e.hardirq>=cpu_s.hardirq) ? (cpu_e.hardirq-cpu_s.hardirq) : cpu_e.hardirq;
        cpu_t.softirq = (cpu_e.softirq>=cpu_s.softirq) ? (cpu_e.softirq-cpu_s.softirq) : cpu_e.softirq;
        cpu_t.steal = (cpu_e.steal>=cpu_s.steal) ? (cpu_e.steal-cpu_s.steal) : cpu_e.steal;
        
	cpu_total = cpu_t.user + cpu_t.nice + cpu_t.sys + cpu_t.idle + \
				cpu_t.iowait + cpu_t.hardirq + cpu_t.softirq + cpu_t.steal;

	if (cpu_total != 0) {
            	cpu_user = (float) (cpu_t.user+cpu_t.nice) / cpu_total;
            	cpu_sys = (float) (cpu_t.sys) / cpu_total;
            	cpu_iowait = (float)(cpu_t.iowait) / cpu_total;
            	cpu_idle = (float)(cpu_t.idle) / cpu_total;
	}

	assert(mod -> col == CPU_MODULE_COL_NUM);

	snprintf((mod->info[0]).index_data, LEN_32, "%.2f", cpu_user*100);
	snprintf((mod->info[1]).index_data, LEN_32, "%.2f", cpu_sys*100);
	snprintf((mod->info[2]).index_data, LEN_32, "%.2f", cpu_iowait*100);
	snprintf((mod->info[3]).index_data, LEN_32, "%.2f", cpu_idle*100);


	cpu_s.user = cpu_e.user;
	cpu_s.nice = cpu_e.nice;
	cpu_s.sys = cpu_e.sys;
	cpu_s.idle = cpu_e.idle;
	cpu_s.iowait = cpu_e.iowait;
	cpu_s.hardirq = cpu_e.hardirq;
	cpu_s.softirq = cpu_e.softirq;
	cpu_s.steal = cpu_e.steal;
	   
	fclose(fd);
}

int
mod_register(struct module* mod)
{
    assert(mod != NULL);

    
    if(-1 == access("/proc/stat", R_OK)) {
            perror("access /proc/stat error:");
            return MODULE_FLAG_NOT_USEABLE;
        }     
	/* TODO: add decide module is usealbe in current HW and SW environment */
	register_module_fields(mod, cpu_mod_info, \
						  CPU_MODULE_COL_NUM, cpu_start, cpu_read);
	return 0;
}
