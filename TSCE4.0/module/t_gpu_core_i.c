#include <stdio.h>
#include "con_define.h"
#include "framework.h"
#include "common.h"
#include "cudainfo.h"
#define GPU_MODULE_COL_NUM  8
#define MAX_GPU_SUPPORT     8

#define MULTIPROCESSORS "Multiprocessors"
#define THREADSPERSM "ThreadsPerMultipro"
#define WARPSIZE "WarpSize"
#define THREADSPERBLOCK "ThreadsPerBlock"
#define REGSPERBLOCK "RegsPerBlock"
#define REGSPERMULTIPRO "RegsPerMultipro"
#define THREADSDIMENSIONS "maxThreadsDim"
#define GRIDDIMENSIONS "maxGridSize"



static int gpu_total_col_num = 0;
static struct mod_info gpu_mod_info[GPU_MODULE_COL_NUM * MAX_GPU_SUPPORT];
int numDev = 0;
static CZDeviceInfo *devInfo = NULL;

static int initial_gpu_info()
{
	if ((numDev = CZCudaDeviceFound()) <= 0)
		return -1;

    gpu_total_col_num = GPU_MODULE_COL_NUM * numDev;
	int i,tmp;
	
    for ( i = 0; i < numDev; i++) 
	{
	    tmp = i * GPU_MODULE_COL_NUM;
    	sprintf(gpu_mod_info[tmp+0].index_hdr, "GPU%d_%s", i, MULTIPROCESSORS);
    	sprintf(gpu_mod_info[tmp+1].index_hdr, "GPU%d_%s", i, THREADSPERSM);
    	sprintf(gpu_mod_info[tmp+2].index_hdr, "GPU%d_%s", i, WARPSIZE);
    	sprintf(gpu_mod_info[tmp+3].index_hdr, "GPU%d_%s", i, THREADSPERBLOCK);
    	sprintf(gpu_mod_info[tmp+4].index_hdr, "GPU%d_%s", i, REGSPERBLOCK);
    	sprintf(gpu_mod_info[tmp+5].index_hdr, "GPU%d_%s", i, REGSPERMULTIPRO);
		sprintf(gpu_mod_info[tmp+6].index_hdr, "GPU%d_%s", i, THREADSDIMENSIONS);
		sprintf(gpu_mod_info[tmp+7].index_hdr, "GPU%d_%s", i, GRIDDIMENSIONS);


    }
    return 0;
}

void gpu_rt_start()
{
	if (devInfo == NULL)
	{
		devInfo = (CZDeviceInfo*)malloc(sizeof(CZDeviceInfo));
	}
}

void gpu_rt_read(struct module *mod)
{
	uint32 i;
	uint32 tmp;
    for ( i = 0; i < numDev; i++) 
	{
	  devInfo->num = i;
	  if (CZCudaReadDeviceInfo(devInfo) < 0)
	  {
		  continue;
	  }
	  
	  tmp = i * GPU_MODULE_COL_NUM;
      snprintf (mod->info[tmp+0].index_data, LEN_32, "%d", devInfo->core.muliProcCount);
      snprintf (mod->info[tmp+1].index_data, LEN_32, "%d", devInfo->core.maxThreadsPerMultiProcessor);
      snprintf (mod->info[tmp+2].index_data, LEN_32, "%d", devInfo->core.SIMDWidth);
      snprintf (mod->info[tmp+3].index_data, LEN_32, "%d", devInfo->core.maxThreadsPerBlock);
      snprintf (mod->info[tmp+4].index_data, LEN_32, "%d", devInfo->core.regsPerBlock);
      snprintf (mod->info[tmp+5].index_data, LEN_32, "%d", devInfo->core.regsPerMultipro);
	  snprintf (mod->info[tmp+6].index_data, LEN_32, "%dx%dx%d", devInfo->core.maxThreadsDim[0],
			devInfo->core.maxThreadsDim[1], devInfo->core.maxThreadsDim[2]);
	  snprintf (mod->info[tmp+7].index_data, LEN_32, "%dx%dx%d", devInfo->core.maxGridSize[0],
			devInfo->core.maxGridSize[1], devInfo->core.maxGridSize[2]);
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
