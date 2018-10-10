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

#define MEMDEVICE_MODULE_COL_NUM 2

static struct mod_info memdevice_info[] = {
	{"mem_installed_num","\0"},
	{"mem_volume","\0"},
};

#define CALC_INTERVAL 300
static unsigned num = 0;


void 
memdevice_start()
{
	//NULL
}


void 
memdevice_read(struct module *mod)
{

	assert(mod != NULL);

	static unsigned int count = 0;
	static unsigned int size = 0;
	char buf[1024] = {0};
	FILE *fp;
	
	if (num++ % CALC_INTERVAL != 0)
	{
		snprintf((mod->info[0]).index_data, LEN_32, "%d", count);
		snprintf((mod->info[1]).index_data, LEN_32, "%d", size);
		return;
	}
	
	count = 0;
	size = 0;
	if ((fp = popen("dmidecode", "r")) == NULL) 
	{
		perror("failed to open dmidecode");
		return;
	}
	
	char flag =  0;
	char msg[20];
	
	while (fgets(buf,1024,fp)) 
	{
		if (strlen(buf) <= 0)
		{
			continue;
		}
		if (!flag)
		{
			if (!strcasecmp(buf, "Memory Device\n"))
			{
				flag = 1;
			}
		}
		else
		{
			//Size: No Module Installed
			//Size: 256 MB
			if (1 == sscanf(buf, "%*[^S]Size: %s%*s", msg))
			{
				if (strcasecmp(msg, "No"))
				{
					count++;
					size += atoi(msg);
				}
				flag = 0;
			}
		}
		memset(buf, 0, sizeof(buf));	
	}
	fclose(fp);
	assert (mod -> col == MEMDEVICE_MODULE_COL_NUM);
	
	snprintf((mod->info[0]).index_data, LEN_32, "%d", count);
	snprintf((mod->info[1]).index_data, LEN_32, "%d", size);
}

int
mod_register(struct module *mod)
{
	assert(mod != NULL);

	FILE *fp;
	char error_buf[LEN_64];

	fp =popen("dmidecode 2>&1", "r");
        fgets(error_buf,LEN_64, fp);

        if (strstr(error_buf, "not found"))
        {
                return MODULE_FLAG_NOT_USEABLE;
        }
        fclose(fp);

	/* TODO: add decide module is useable in current HW and SW environment */

	register_module_fields(mod, memdevice_info, MEMDEVICE_MODULE_COL_NUM, \
				 memdevice_start, memdevice_read);
	return 0;
}
