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

#define SYSLOAD_MODULE_COL_NUM 3

static struct mod_info sysload_info[] = {
	{"load_one","\0"},
	{"load_five","\0"},
	{"load_fifteen", "\0"}
};


void 
sysload_start()
{
	//NULL
}


void 
sysload_read(struct module *mod)
{

	assert(mod != NULL);

	char buf[1024] = {0};
	FILE *fp;
	
	if ( (fp = fopen("/proc/loadavg", "r")) == NULL) {
		printf("failed to open - /proc/loadavg");
		return;
	}

	if (fgets(buf,1024,fp) == NULL) {
		printf("fgets /proc/loadavg buf = 0\n");
		fclose(fp);
		return;
	}
        fclose(fp);
	
	assert (mod -> col == SYSLOAD_MODULE_COL_NUM);
	
	sscanf(buf, "%s%s%s%*", (mod->info [0]).index_data, (mod->info [1]).index_data, (mod->info [2]).index_data);
}

int
mod_register(struct module *mod)
{
	assert(mod != NULL);

	 if(-1 == access("/proc/loadavg", R_OK)) {
            perror("access /proc/loadavg error:");
            return MODULE_FLAG_NOT_USEABLE;
        }     

	/* TODO: add decide module is useable in current HW and SW environment */

	register_module_fields(mod, sysload_info, SYSLOAD_MODULE_COL_NUM, \
				 sysload_start, sysload_read);
	return 0;
}
