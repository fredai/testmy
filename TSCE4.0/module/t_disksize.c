/*
 * Copyright (C) Inspur(Beijing)
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/statvfs.h>

#include "con_define.h"
#include "framework.h"
#include "common.h"

#define DISK_SIZE_MODULE_COL_NUM 2

static struct mod_info disk_size_info[] = {
	{"disk_maxused","\0"},
	{"mounted", "\0"}
};


void 
disk_size_start()
{
	//NULL
}


void 
disk_size_read(struct module *mod)
{

	assert(mod != NULL);

	char buf[1024] = {0};
	FILE *fp;
	
	if ((fp = fopen("/etc/mtab", "r")) == NULL) 
	{
		printf("failed to open /etc/mtab\n");
		return;
	}

	char path[256] = {0};
	char mounted[256] = {0};
	float used = 0;
	float tmp_used = 0;
	struct statvfs stat;
	
	while (fgets(buf, 1024, fp)) 
	{
		do
		{
			///dev/sda2 / ext4 rw 0 0
			if (strlen(buf) > 0 && sscanf(buf, "%*s%s", path) == 1)
			{
				if(statvfs(path,&stat) <0)
				{
					printf("%s call statvfs() failed\n", path);
					break;
				}
				if (stat.f_blocks > 0)
				{
					tmp_used = stat.f_blocks - stat.f_bavail;
					tmp_used =  tmp_used / stat.f_blocks;
					if (tmp_used > used)
					{
						used = tmp_used;
						strncpy(mounted, path, sizeof(mounted)-1);
					}
				}
			}
		} while (0);
		memset(buf, 0, sizeof(buf));
		memset(path, 0, sizeof(path));
	}
	fclose(fp);
	assert (mod -> col == DISK_SIZE_MODULE_COL_NUM);
	
	snprintf((mod->info[0]).index_data, LEN_32, "%d", (int)(used*100));
	snprintf((mod->info[1]).index_data, LEN_32, "%s", mounted);
}

int
mod_register(struct module *mod)
{
	assert(mod != NULL);

	/* TODO: add decide module is useable in current HW and SW environment */

	register_module_fields(mod, disk_size_info, DISK_SIZE_MODULE_COL_NUM, \
				 disk_size_start, disk_size_read);
	return 0;
}
