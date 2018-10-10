
/*
 * Copyright (C) Inspur(Bejing)
 * 
 */


#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/vfs.h>
#include <fcntl.h>

#include "con_define.h"
#include "framework.h"
#include "common.h"
#include "cpuinfo.c"

#define DISK_VOLUME_PATH    "/etc/mtab"
#define MOUNT_DEV                   "/dev/"
#define OS_PATH "/etc/system-release"
#define OS_PATH_UBUNTU "/etc/issue"

#define STATIC_MODULE_COL_NUM 4

static int s_is_flag = 0;

static struct mod_info static_mod_info [] = {
	{"cpu_type", "\0"},
	{"cpu_cores", "\0"},
	{"os_type", "\0"},
	{"disk_volume",  "\0"}
};


struct  t_static
{
  char cpu_type[256];
  int cpu_cores;
  char os_type[256];
  unsigned long long disk_volume;
};

struct  t_static static_info;

void 
sys_start()
{
	
}


int get_OS_type(char type[], int number)
{
	FILE *fp = NULL;
	char buf[256] = "";
	char name[8] = "";
	char version[24] = "";
	char *p = NULL;

	fp = fopen(OS_PATH, "r");
	if (NULL == fp) {
		perror("fopen OS_PATH");	
//		return -1;
//		for Ubuntu
		fp = fopen(OS_PATH_UBUNTU, "r");
	}
	if (NULL == fgets(buf, sizeof(buf), fp)) {
		perror("fgets OS_PATH");	
		return -1;
	}
	
	if (strlen(buf) <= sizeof(buf)) {
		buf[strlen(buf) -1] = 0;	
	}

	if (NULL != (p = strstr(buf,"release"))) {
              sscanf(p+sizeof("release"), "%s %*s", version);
              if (strstr(buf,"Red Hat Enterprise Linux")) {
                  strncpy(name, "RHEL", sizeof(name));
              } else if (strstr(buf, "CentOS")) {
                  strncpy(name, "CentOS", sizeof(name));
	      }
        } else if(NULL != (p = strstr(buf,"Ubuntu"))) {
	      sscanf(p+sizeof("Ubuntu"), "%s %*s", version);
	      strncpy(name, "Ubuntu", sizeof(name));
	}

        if (0 == strcmp(name, "")) {
             snprintf(type, number, "%s", buf);
        } else {
             snprintf(type, number, "%s %s", name, version);
        }

	fclose(fp);
	return 0;
}

int get_disk_volume(void)
{
	FILE *fp = NULL;
	char buf[256] = "";
	struct statfs stat;
	char disk_dir[256] = "";

	fp = fopen(DISK_VOLUME_PATH, "r");
	if (NULL == fp) {
            perror("fopen DISK_VOLUME_PATH");
            return -1;
	}
	while (NULL != fgets(buf, sizeof(buf), fp)) {
            if (0 == strncmp(buf, MOUNT_DEV, strlen(MOUNT_DEV))) {
                bzero(&disk_dir, sizeof(disk_dir));
                bzero(&stat, sizeof(struct statfs));
                sscanf(buf, "%*s %s", disk_dir);
                if (-1 == statfs(disk_dir, &stat)) {
                    perror("statfs disk_dir");
                    fclose(fp);
                    return -1;
                }
                static_info.disk_volume += (unsigned long long)(stat.f_bsize * stat.f_blocks) / (1000 * 1000);
                printf("disk_dir:%s, all:%llu, block:%llu, blocks:%llu\n", disk_dir, static_info.disk_volume, (unsigned long long)stat.f_bsize, (unsigned long long)stat.f_blocks);
            }
	}
	fclose(fp);
	return 0;
}

void 
sys_read(struct module *mod)
{
    assert(mod);
    
    if (0 == s_is_flag) {
	int cores_number = 0;

	/* get number of cores */
	cores_number = sysconf(_SC_NPROCESSORS_CONF);
	if (cores_number < 0) {
		perror("sysconf(_SC_NPROCESSORS_CONF) fail");
		static_info.cpu_cores = -1;
		return;
		
	} else {
	    static_info.cpu_cores = cores_number;
	}

	/* get cpu type */
	get_cpu_model(static_info.cpu_type, sizeof(static_info.cpu_type));
	
	/* get OS type */
	get_OS_type(static_info.os_type, sizeof(static_info.os_type));

	/* get disk volume */
	get_disk_volume();

	s_is_flag = 1;
    }
    
    snprintf((mod->info[0]).index_data, LEN_32, "%s", static_info.cpu_type);
    snprintf((mod->info[1]).index_data, LEN_32, "%d", static_info.cpu_cores);
    snprintf((mod->info[2]).index_data, LEN_32, "%s", static_info.os_type);
    snprintf((mod->info[3]).index_data, LEN_32, "%llu", static_info.disk_volume);

    //get_cpuinfo();
    
}


int
mod_register(struct module* mod)
{
    assert(mod);

	/* TODO: add decide module is usealbe in current HW and SW environment */
        if (-1 == access(DISK_VOLUME_PATH, R_OK)) {
            perror("access DISK_VOLUME_PATH");
	    return MODULE_FLAG_NOT_USEABLE;
        }
	memset(&static_info, 0, sizeof(static_info));
	register_module_fields(mod, static_mod_info, \
						  STATIC_MODULE_COL_NUM, sys_start, sys_read);
	
	return 0;
}
