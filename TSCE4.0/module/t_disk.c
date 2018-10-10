
/*
 * Copyright (C) Inspur(Bejing)
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdarg.h>
#include <limits.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>
#include <errno.h>

#include "con_define.h"
#include "framework.h"
#include "common.h"

#define DISK_MODULE_COL_NUM         4
#define SYS_BLOCK_DIR "/sys/block/"
#define SCSI_DISK "sd"
#define DISK_STAT "stat"

unsigned long long s_millisecond;

struct  t_disk
{
  char disk_name[32];
 unsigned long long reads;
 unsigned long long rkb;    
 unsigned long long writes;
 unsigned long long wkb;
};


struct  c_disk
{
  char disk_name[32];
  float reads;
  float r_size;    
  float writes;     //MB
  float w_size;   //MB
};

static struct mod_info disk_mod_info [] = {
	{"disk_read", "\0"},
	{"disk_rsize", "\0"},
	{"disk_write", "\0"},
	{"disk_wsize",  "\0"}
};


struct disk_list
{
    char disk_sys_path[32];
    struct  t_disk s_disk;
    struct  t_disk e_disk;
    struct  c_disk disk;
    struct disk_list *next;
};

struct disk_list *dl_head = NULL;
struct disk_list *dl_rear = NULL;

void del_list()
{
	struct disk_list *p = dl_head;
	struct disk_list *next = NULL;

	while ( p != NULL) {
		next = p->next;
		printf("%s\n", p->disk_sys_path);
		free(p);
		p = next;
	}
	dl_head = NULL;
	dl_rear = NULL;
}

int insert_list(char *sd_path)
{
    struct disk_list *p = NULL;

    p = (struct disk_list *)malloc(sizeof(struct disk_list));
    if (NULL == p) {
        perror("malloc sd* disk node failed");
        return -1;
    }
    strncpy(p->disk_sys_path, sd_path, sizeof(p->disk_sys_path));
    p->next = NULL;

    if (NULL == dl_head) {
        dl_head = p;
        dl_rear = p;
    } else {
        dl_rear->next = p;
        dl_rear = p;
    }
    return 0;
}


int find_non_usb_disk(void)
{
	struct dirent *entry = NULL; 
	char fullpath[64] = {0};
	char tmppath[64] = {0};
	char buf[256] = {0};
	char *ptr = NULL;
	char *bname = NULL;
	char *dir = NULL;
	DIR  *dp = NULL;

	dp = opendir(SYS_BLOCK_DIR); 
	if (NULL == dp) {
		perror(SYS_BLOCK_DIR);
		return -1;
	}

	strncpy(fullpath, SYS_BLOCK_DIR, sizeof(fullpath));
	ptr = fullpath + strlen(SYS_BLOCK_DIR);
	*ptr = 0;

	while (NULL != (entry = readdir(dp))) {
		if (!strncmp((entry->d_name), "sd", strlen("sd"))) {

			strcpy(ptr, entry->d_name);
			int fd = open(fullpath, O_RDONLY);
			if (-1 == fd) {
				perror(fullpath);
				return -1;
			}
			sprintf(tmppath, "%s/%d", "/proc/self/fd", fd);
			if (1 < readlink(tmppath,buf,sizeof(buf))) {
				dir = buf;
				do {
					bname = basename(dir);
					dir = dirname(dir);
					if ((!strncmp(bname, "usb", strlen("usb")))) {
							break;
					}
				}while (*bname != '/');

				if (*bname == '/') {
					snprintf(tmppath, sizeof(tmppath), "%s/%s", fullpath, DISK_STAT);
					insert_list(tmppath);
				} else {
				
					printf("%s is usb disk\n", entry->d_name );
				} 
				*ptr = 0;
			}
			close(fd);
		}
	}
	closedir(dp);
	return 0;	
}




void
disk_start()
{
    FILE *fp = NULL;
    char buf[1024] = {0};
    int ret = 0;
    struct disk_list *p = dl_head;

    s_millisecond = get_current_millisecond();

    while (NULL != p) {
        if( NULL == (fp = fopen(p->disk_sys_path,"r"))) 
        {
            perror(p->disk_sys_path);
            return;
        }

        if(NULL == fgets(buf,1024,fp)) {
            perror(p->disk_sys_path);
            fclose(fp);
            p = p->next;
            continue;
        }

        /* 
          *  1--number of issued reads
          *  3--number of sectors read
          *  5--number of writes completed
          *  7--number of sectors written
          */
          /**************** 1   2   3     4    5    6   7 *********/
            ret = sscanf(&buf[0], "%llu %*s %llu %*s %llu %*s %llu",
                                                    &p->s_disk.reads,
                                                    &p->s_disk.rkb,
                                                    &p->s_disk.writes,
                                                    &p->s_disk.wkb);
                                                    
         p->s_disk.rkb /= 2048; /* sectors = 512 bytes */
         p->s_disk.wkb /= 2048;
        
         fclose(fp);
         p = p->next;
    }
    
    return;
}


void
disk_read(struct module *mod)
{
    FILE *fp = NULL;
    char buf[1024] = {0};
    int ret = 0;
    struct disk_list *p = dl_head;
    unsigned long long milliseconds = 0;
    unsigned long long time = 0;
    struct c_disk disk_sum = {{0}, 0.0, 0.0, 0.0, 0.0};

    milliseconds = get_current_millisecond();

    while (NULL != p) {
        if( NULL == (fp = fopen(p->disk_sys_path,"r"))) 
        {
            perror(p->disk_sys_path);
            return;
        }

        if(NULL == fgets(buf,1024,fp)) {
            perror(p->disk_sys_path);
            fclose(fp);
            p = p->next;
            continue;
        }

        /* 
          *  1--number of issued reads
          *  3--number of sectors read
          *  5--number of writes completed
          *  7--number of sectors written
          */
          /**************** 1   2   3     4    5    6   7 *********/
            ret = sscanf(&buf[0], "%llu %*s %llu %*s %llu %*s %llu",
                                                    &p->e_disk.reads,
                                                    &p->e_disk.rkb,
                                                    &p->e_disk.writes,
                                                    &p->e_disk.wkb);
                                                    
         p->e_disk.rkb /= 2048; /* sectors = 512 bytes */
         p->e_disk.wkb /= 2048;

        p->disk.r_size = (float)((p->e_disk.rkb >= p->s_disk.rkb) ? (p->e_disk.rkb - p->s_disk.rkb) : p->e_disk.rkb);
        p->disk.reads = (p->e_disk.reads > p->s_disk.reads) ? p->disk.r_size/(float)(p->e_disk.reads - p->s_disk.reads):0;
        p->disk.w_size = (float)((p->e_disk.wkb >= p->s_disk.wkb) ? (p->e_disk.wkb - p->s_disk.wkb) : p->e_disk.wkb);
        p->disk.writes = (p->e_disk.writes > p->s_disk.writes) ? p->disk.w_size/(float)(p->e_disk.writes - p->s_disk.writes):0;

        disk_sum.r_size += p->disk.r_size;
        disk_sum.reads += p->disk.reads;
        disk_sum.w_size += p->disk.w_size;
        disk_sum.writes += p->disk.writes;

        p->s_disk.reads = p->e_disk.reads;
        p->s_disk.rkb = p->e_disk.rkb;
        p->s_disk.writes = p->e_disk.writes;
        p->s_disk.wkb = p->e_disk.wkb;

        //printf("%s\t%s\n", p->disk_sys_path, buf);
        //printf("\t\t%llu\t%llu\t%llu\t%llu\n", p->e_disk.reads, p->e_disk.rkb, p->e_disk.writes, p->e_disk.wkb);
        
         fclose(fp);
         p = p->next;
    }

    
    time = ((milliseconds >= s_millisecond) ? (milliseconds - s_millisecond) : milliseconds);
    if (0 != time) {
        time /= 1000.0;
        snprintf((mod->info[0]).index_data, LEN_32, "%.2f", disk_sum.r_size / time);
        snprintf((mod->info[1]).index_data, LEN_32, "%.2f", disk_sum.reads);
        snprintf((mod->info[2]).index_data, LEN_32, "%.2f", disk_sum.w_size / time);
        snprintf((mod->info[3]).index_data, LEN_32, "%.2f", disk_sum.writes);
    } else {
        printf("millisecnods, divisor == 0\n");
        snprintf((mod->info[0]).index_data, LEN_32, "%.2f", 0.0);
        snprintf((mod->info[1]).index_data, LEN_32, "%.2f", 0.0);
        snprintf((mod->info[2]).index_data, LEN_32, "%.2f", 0.0);
        snprintf((mod->info[3]).index_data, LEN_32, "%.2f", 0.0);
    }
    s_millisecond = milliseconds;
     
    return;
}

int
mod_register(struct module* mod)
{
    assert(mod != NULL);
        /* check the permissions of the "/proc/diskstats"*/
        if(-1 == find_non_usb_disk()) {
                 del_list();
		return MODULE_FLAG_NOT_USEABLE;
	}
    
	/* TODO: add decide module is usealbe in current HW and SW environment */
	register_module_fields(mod, disk_mod_info, \
						  DISK_MODULE_COL_NUM, disk_start, disk_read);
	
	return 0;
}
