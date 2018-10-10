
/*
 * Copyright (C) Inspur(Beijing)
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "con_define.h"
#include "framework.h"
#include "common.h"

#define MEM_MODULE_COL_NUM 6

struct  t_mem
{
	char name[32];
	unsigned long long size;
} s_mem;

static struct mod_info mem_mod_info[] = {
	{"mem_total","\0"},
	{"mem_used","\0"},
	{"buffer", "\0"},
	{"cached", "\0"},
	{"mem_free", "\0"},
	{"mem_ratio", "\0"}
};


void 
mem_start()
{
	//NULL
}


void 
mem_read(struct module *mod)
{

	assert(mod != NULL);

#if 0
	float mem_total = 0;
	float mem_used = 0;
	float buffers = 0;
	float cached = 0;
#endif
	unsigned int mem_total = 0;
	unsigned int mem_used = 0;
	unsigned int buffers = 0;
	unsigned int cached = 0;
	unsigned int  mem_free = 0;
	float  mem_ratio = 0;

	char buf[1024];
	int ret;
	FILE *mem_fp;
	
	if ( (mem_fp = fopen("/proc/meminfo", "r")) == NULL) {
		printf("failed to open - /proc/meminfo");
		return;
	}

	/*
	MemTotal:       49420868 kB
	MemFree:        35426732 kB
	Buffers:           24176 kB
	Cached:         12386040 kB
	SwapCached:        16844 kB
	*/
	while (1) {
		if (fgets(buf,1024,mem_fp) == NULL) {
			break;
		}
                                    
		ret = sscanf(&buf[0], "%s %llu", (char *)&s_mem.name, &s_mem.size);
		/* printf("%s %llu\n", s_mem.name, s_mem.size); */
		if (!strcmp(s_mem.name,"MemTotal:")) {
			mem_total = s_mem.size;
        	}
		else if(!strcmp(s_mem.name,"MemFree:")) {
			/* mem_used = s_teye.mem.MemTotal-s_mem.size; */
			mem_free = s_mem.size;
		}
		else if(!strcmp(s_mem.name,"Buffers:")) {
			buffers = s_mem.size;
		}
		else if(!strcmp(s_mem.name,"Cached:")) {
			cached = s_mem.size;
			break;
        	}
	} 
	mem_used = mem_total - mem_free - buffers - cached;
	mem_ratio = (mem_used/(float)mem_total)*100.0;
	fclose(mem_fp);

	assert (mod -> col == MEM_MODULE_COL_NUM);

	snprintf ( (mod->info [0]).index_data, LEN_32, "%.2f", mem_total / 1024.0);
	snprintf ( (mod->info [1]).index_data, LEN_32, "%.2f", mem_used / 1024.0);
	snprintf ( (mod->info [2]).index_data, LEN_32, "%.2f", buffers / 1024.0);
	snprintf ( (mod->info [3]).index_data, LEN_32, "%.2f", cached / 1024.0);
	snprintf ( (mod->info [4]).index_data, LEN_32, "%.2f", mem_free / 1024.0);
	snprintf ( (mod->info [5]).index_data, LEN_32, "%.2f", mem_ratio);
}


int
mod_register(struct module *mod)
{
	assert(mod != NULL);

	 if(-1 == access("/proc/meminfo", R_OK)) {
            perror("access /proc/meminfo error:");
            return MODULE_FLAG_NOT_USEABLE;
        }     

	/* TODO: add decide module is useable in current HW and SW environment */

	register_module_fields(mod, mem_mod_info, MEM_MODULE_COL_NUM, \
				 mem_start, mem_read);
	return 0;
}
