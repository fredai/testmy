#include <stdio.h>
#include <pthread.h>
#include "con_define.h"
#include "framework.h"
#include "common.h"
#include "cudainfo.h"
#define GPU_MODULE_COL_NUM  5
#define MAX_GPU_SUPPORT     8

#define COPYHDPIN "CopyHDPin"
#define COPYHDPAGE "CopyHDPage"
#define COPYDHPIN "CopyDHPin"
#define COPYDHPAGE "CopyDHPage"
#define COPYDD "CopyDD"



static int gpu_total_col_num = 0;
static struct mod_info gpu_mod_info[GPU_MODULE_COL_NUM * MAX_GPU_SUPPORT];
int numDev = 0;
static CZDeviceInfo *devInfo = NULL;
static char *done = NULL;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 

static void* thread_proc(void *args)
{
  int idx = *((int*)args);
  while (1)
  {
    pthread_mutex_lock(&mutex);  
    pthread_cond_wait(&cond, &mutex);  
    pthread_mutex_unlock(&mutex);
	if (done[idx] == 0)
    {
    	CZCudaCalcDeviceBandwidth(&devInfo[idx]);
    	done[idx] = 1;
	}
  }
  return NULL;
}

int create_threads(int num)
{
  pthread_t tid;
  int i;
  for (i = 0; i < num; ++i)
  {
    if (pthread_create(&tid, NULL, thread_proc, &devInfo[i].num) != 0)
    {
      printf("gpu_mem_cpy create thread[%d] failed\n", i);
      return -1;
    }
  }
  return 0;
}

static int initial_gpu_info()
{
  if ((numDev = CZCudaDeviceFound()) <= 0)
    return -1;

  gpu_total_col_num = GPU_MODULE_COL_NUM * numDev;
  int i,tmp;

  for ( i = 0; i < numDev; i++)
  {
    tmp = i * GPU_MODULE_COL_NUM;
    sprintf(gpu_mod_info[tmp+0].index_hdr, "GPU%d_%s", i, COPYHDPIN);
    sprintf(gpu_mod_info[tmp+1].index_hdr, "GPU%d_%s", i, COPYHDPAGE);
    sprintf(gpu_mod_info[tmp+2].index_hdr, "GPU%d_%s", i, COPYDHPIN);
    sprintf(gpu_mod_info[tmp+3].index_hdr, "GPU%d_%s", i, COPYDHPAGE);
    sprintf(gpu_mod_info[tmp+4].index_hdr, "GPU%d_%s", i, COPYDD);
  }
  if (devInfo == NULL)
  {
 	devInfo = (CZDeviceInfo*)malloc(sizeof(CZDeviceInfo)*numDev);
	int i;
	for (i = 0; i < numDev; i++)
	{
		memset(&devInfo[i], 0, sizeof(CZDeviceInfo));
		devInfo[i].num = i;
		CZCudaReadDeviceInfo(&devInfo[i]);
	}
	done = (char*)malloc(numDev);
  }
  return create_threads(numDev);
}

void gpu_rt_start()
{
	sleep(2);
}

void gpu_rt_read(struct module *mod)
{
  memset(done, 0, sizeof(numDev));
  pthread_mutex_lock(&mutex);
  pthread_cond_broadcast(&cond);  
  pthread_mutex_unlock(&mutex);
  uint32 i;
  uint32 tmp;
  unsigned char sum = 0;
  while (sum < numDev)
  {
    usleep(10000);
    for ( i = 0; i < numDev; i++)
    {
      if (done[i] == 1)
      {
        tmp = i * GPU_MODULE_COL_NUM;
        snprintf (mod->info[tmp+0].index_data, LEN_32, "%.3f", devInfo[i].band.copyHDPin/1024/1024);
        snprintf (mod->info[tmp+1].index_data, LEN_32, "%.3f", devInfo[i].band.copyHDPage/1024/1024);
        snprintf (mod->info[tmp+2].index_data, LEN_32, "%.3f", devInfo[i].band.copyDHPin/1024/1024);
        snprintf (mod->info[tmp+3].index_data, LEN_32, "%.3f", devInfo[i].band.copyDHPage/1024/1024);
        snprintf (mod->info[tmp+4].index_data, LEN_32, "%.3f", devInfo[i].band.copyDD/1024/1024);
        done[i] = 0;
        ++sum;
      }
    }
  }
}


int
mod_register(struct module* mod)
{
  assert(mod != NULL);
  if (-1 == initial_gpu_info())
  {
    return MODULE_FLAG_NOT_USEABLE;
  }

  // TODO: add decide module is usealbe in current HW and SW environment
  register_module_fields(mod, gpu_mod_info, \
    gpu_total_col_num, gpu_rt_start, gpu_rt_read);
  return 0;
}
