#include <stdio.h>
#include "con_define.h"
#include "framework.h"
#include "common.h"
#include "cudainfo.h"
#define GPU_MODULE_COL_NUM  8
#define MAX_GPU_SUPPORT     8

#define L2CACHESIZE "L2CacheSize"
#define SHAREDPERBLOCK "SharedPerBlock"
#define SHAREDPERSM "SharedPerMultiPro"
#define PITCH "Pitch"
#define TOTALCON "TotalConstant"
#define GPUOVERLAP "GPUOverlap"
#define MAPHOSTMEM "MapHostMem"
#define UNIFIEDADDR "UnifiedAddr"



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
    	sprintf(gpu_mod_info[tmp+0].index_hdr, "GPU%d_%s", i, L2CACHESIZE);
    	sprintf(gpu_mod_info[tmp+1].index_hdr, "GPU%d_%s", i, SHAREDPERBLOCK);
    	sprintf(gpu_mod_info[tmp+2].index_hdr, "GPU%d_%s", i, SHAREDPERSM);
    	sprintf(gpu_mod_info[tmp+3].index_hdr, "GPU%d_%s", i, PITCH);
    	sprintf(gpu_mod_info[tmp+4].index_hdr, "GPU%d_%s", i, TOTALCON);
    	sprintf(gpu_mod_info[tmp+5].index_hdr, "GPU%d_%s", i, GPUOVERLAP);
		sprintf(gpu_mod_info[tmp+6].index_hdr, "GPU%d_%s", i, MAPHOSTMEM);
		sprintf(gpu_mod_info[tmp+7].index_hdr, "GPU%d_%s", i, UNIFIEDADDR);

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
      snprintf (mod->info[tmp+0].index_data, LEN_32, "%d", devInfo->mem.l2CacheSize/1024/1024);
      snprintf (mod->info[tmp+1].index_data, LEN_32, "%d", devInfo->mem.sharedPerBlock/1024);
      snprintf (mod->info[tmp+2].index_data, LEN_32, "%d", devInfo->mem.sharedPerMultiProcessor/1024);
      snprintf (mod->info[tmp+3].index_data, LEN_32, "%d", devInfo->mem.maxPitch/1024/1024);
      snprintf (mod->info[tmp+4].index_data, LEN_32, "%d", devInfo->mem.totalConst/1024);
	  snprintf (mod->info[tmp+5].index_data, LEN_32, "%s", devInfo->mem.gpuOverlap? "Yes": "No");
	  snprintf (mod->info[tmp+6].index_data, LEN_32, "%s", devInfo->mem.mapHostMemory? "Yes": "No");
	  snprintf (mod->info[tmp+7].index_data, LEN_32, "%s", devInfo->mem.unifiedAddressing? "Yes": "No");
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
