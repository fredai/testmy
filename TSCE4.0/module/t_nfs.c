
/*
 * Copyright (C) Inspur(Bejing)
 * 
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include "con_define.h"
#include "framework.h"
#include "common.h"

#define NFS_MODULE_COL_NUM 4
#define NFS_FILE "/proc/net/rpc/nfs"
#define NFSD_FILE "/proc/net/rpc/nfsd"


struct t_nfs {
  unsigned long long read;
  unsigned long long write;
  unsigned long long read_nfsd;  // MB/s
  unsigned long long write_nfsd; // MB/s
  unsigned long long milliseconds;
};
struct  c_nfs
{
  unsigned long long read;
  unsigned long long write;
  float read_nfsd;  // MB/s
  float write_nfsd; // MB/s
};

static struct mod_info nfs_mod_info [] = {
	{"nfs_read", "\0"},
	{"nfs_write", "\0"},
	{"nfs_server_read", "\0"},
	{"nfs_server_write",  "\0"}
};



struct t_nfs s_nfs,s_nfs_v3; struct t_nfs e_nfs,e_nfs_v3;
static int if_nfs = 1;
static int if_nfsd = 1;

void 
nfs_start()
{
  FILE *nfs_fp = NULL;
  FILE *nfsd_fp = NULL;
  char buf[1024];

  memset(&s_nfs, 0, sizeof(s_nfs));
  
  /* Read data from NFS client */
  nfs_fp =  fopen(NFS_FILE, "r");
  if (NULL == nfs_fp) {
	perror("fopen NFS_FILE");
    if_nfs = 0;
  }

  s_nfs.milliseconds = get_current_millisecond();
  
  if (if_nfs != 0){
          while (0 != fgets(buf, 1024, nfs_fp)) {
            if (0 == strncmp(buf, "proc4", sizeof("proc4") - 1)) {
              sscanf(buf, "proc4 %*s %*s %Lu %Lu", &s_nfs.read, &s_nfs.write);
            }
            if (0 == strncmp(buf, "proc3", sizeof("proc3") - 1)) {
              sscanf(buf, "proc3 %*s %*s %*s %*s %*s %*s %*s %Lu %Lu", &s_nfs_v3.read, &s_nfs_v3.write);
            }
          }

          fclose(nfs_fp);
  }

  /* Read data from NFS server */
  nfsd_fp = fopen(NFSD_FILE, "r");
  if (NULL == nfsd_fp) {
	perror("fopen NFSD_FILE");
    if_nfsd = 0;
  }
    if (nfsd_fp != 0) {
      while (0 != fgets(buf, 1024, nfsd_fp)) {
        if (0 == strncmp(buf, "io ", sizeof("io ") - 1)) {
          sscanf(buf, "io %llu %llu", &s_nfs.read_nfsd, &s_nfs.write_nfsd);
    	  break;
        }
      }

      fclose(nfsd_fp);
  }
}



void 
nfs_read(struct module *mod)
{
	assert(mod != NULL);

  char buf[1024];
  FILE *nfs_fp = NULL;
  FILE *nfsd_fp = NULL;
  double nfs_read = 0;
  double nfs_read_v3 = 0;
  double nfs_write = 0;
  double nfs_write_v3 = 0;
  double nfsd_read = 0;
  double nfsd_write = 0;
  double seconds = 0.0;
  unsigned long long milliseconds = 0;

  nfs_fp = fopen(NFS_FILE, "r");
  nfsd_fp = fopen(NFSD_FILE, "r");

  if (NULL == nfs_fp) {
		perror("fopen NFS_FILE");
		if_nfs = 0;
		
		nfs_read = 0.0;
		nfs_read_v3 = 0.0;
                nfs_write = 0.0;
                nfs_write_v3 = 0.0;
		s_nfs.read = 0;
		s_nfs_v3.read = 0;
		s_nfs.write = 0;
		s_nfs_v3.write = 0;
  } else {
        if_nfs = 1;
  }
  
  if (NULL == nfsd_fp) {
	    perror("fopen NFSD_FILE");
	    if_nfsd = 0;
	    
	    nfsd_read = 0.0;
             nfsd_write = 0.0;
	    s_nfs.read_nfsd = 0;
             s_nfs.write_nfsd = 0;
  } else {
        if_nfsd = 1;
  }

  e_nfs.milliseconds = get_current_millisecond();

  /* For NFS client */
  if (0 != if_nfs) {
    while (0 != fgets(buf, 1024, nfs_fp)) {
      if (0 == strncmp(buf, "proc4", sizeof("proc4") - 1)) {
    	sscanf(buf, "proc4 %*s %*s %llu %llu", &e_nfs.read, &e_nfs.write);
      }
      if (0 == strncmp(buf, "proc3", sizeof("proc3") - 1)) {
      	sscanf(buf, "proc3 %*s %*s %*s %*s %*s %*s %*s %Lu %Lu", &e_nfs_v3.read, &e_nfs_v3.write);
      }
    }
    
    fclose(nfs_fp);
    
    nfs_read = (e_nfs.read >= s_nfs.read) ? (e_nfs.read - s_nfs.read) : e_nfs.read;
    nfs_read_v3 = (e_nfs_v3.read >= s_nfs_v3.read) ? (e_nfs_v3.read - s_nfs_v3.read) : e_nfs_v3.read;
    nfs_write = (e_nfs.write >= s_nfs.write) ? (e_nfs.write - s_nfs.write) : e_nfs.write;
    nfs_write_v3 = (e_nfs_v3.write >= s_nfs_v3.write) ? (e_nfs_v3.write - s_nfs_v3.write) : e_nfs_v3.write;
    /* Check data to see whether they are reasonable */
    if (nfs_read > 10000.0 || nfs_read_v3 > 10000.0)
      nfs_read = 0.0;
    if (nfs_write > 10000.0 || nfs_write_v3 > 10000.0)
      nfs_write = 0.0;
        
    s_nfs.read = e_nfs.read;
    s_nfs_v3.read = e_nfs_v3.read;
    s_nfs.write = e_nfs.write;
    s_nfs_v3.write = e_nfs_v3.write;
  }

  /* For NFS server */
  if (0 != if_nfsd) {
    while (0 != fgets(buf, 1024, nfsd_fp)) {
      if (0 == strncmp(buf, "io ", sizeof("io ") - 1)) {
    sscanf(buf, "io %llu %llu", &e_nfs.read_nfsd, &e_nfs.write_nfsd);
      }
    }
    
    fclose(nfsd_fp);
    
    nfsd_read = ((e_nfs.read_nfsd >= s_nfs.read_nfsd) ? (e_nfs.read_nfsd - s_nfs.read_nfsd) : e_nfs.read_nfsd) / 1024.0 / 1024.0;
    nfsd_write = ((e_nfs.write_nfsd >= s_nfs.write_nfsd) ? (e_nfs.write_nfsd - s_nfs.write_nfsd) : e_nfs.write_nfsd) / 1024.0 / 1024.0;
    /* Check data to see whether they are reasonable */
    if (nfsd_read > 10000.0)
       nfsd_read = 0;
    if (nfsd_write > 10000.0)
      nfsd_write = 0;
    
    s_nfs.read_nfsd = e_nfs.read_nfsd;
    s_nfs.write_nfsd = e_nfs.write_nfsd;
  }

  milliseconds = (e_nfs.milliseconds > s_nfs.milliseconds) ? (e_nfs.milliseconds - s_nfs.milliseconds) : e_nfs.milliseconds;
  s_nfs.milliseconds = e_nfs.milliseconds;

  assert(mod->col == NFS_MODULE_COL_NUM);
  
  if (0 != milliseconds) {
        seconds = milliseconds / 1000.0;
        snprintf(mod->info[0].index_data, LEN_32, "%.2f", (nfs_read + nfs_read_v3) / seconds);
        snprintf(mod->info[1].index_data, LEN_32, "%.2f", (nfs_write + nfs_write_v3) / seconds);
        snprintf(mod->info[2].index_data, LEN_32, "%.2f", nfsd_read / seconds);
        snprintf(mod->info[3].index_data, LEN_32, "%.2f", nfsd_write / seconds);
    } else {
        printf("secnods, divisor == 0\n");
        snprintf(mod->info[0].index_data, LEN_32, "%.2f", 0.0);
        snprintf(mod->info[1].index_data, LEN_32, "%.2f", 0.0);
        snprintf(mod->info[2].index_data, LEN_32, "%.2f", 0.0);
        snprintf(mod->info[3].index_data, LEN_32, "%.2f", 0.0);
    }
  
}




int
mod_register(struct module* mod)
{
    assert(mod);

    /* check resources that get the nfs items */
    if(-1 == access(NFSD_FILE, R_OK)) {
		perror("access NFSD_FILE");
		//return -1;
	}
	if(-1 == access(NFS_FILE, R_OK)) {
		perror("access NFS_FILE");
		return MODULE_FLAG_NOT_USEABLE;
	}     

	/* TODO: add decide module is usealbe in current HW and SW environment */
	register_module_fields(mod, nfs_mod_info, \
						  NFS_MODULE_COL_NUM, nfs_start, nfs_read);
	return 0;
}
