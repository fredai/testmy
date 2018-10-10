#include <stdio.h>
#include "con_define.h"
#include "framework.h"
#include "common.h"
#include "cudainfo.h"
#define GPU_MODULE_COL_NUM  5
#define MAX_GPU_SUPPORT     8

#define COMPUTECAP	"ComputeCap"
#define COMPUTEMODE "ComputeMode"
#define KERNELEXETIMEOUTENABLE "KernelExecTimeoutEnabled"
#define CONKERNELS "ConcurrentKernels"
#define STREAMPRIOR "StreamPriorities"



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
	    sprintf(gpu_mod_info[tmp+0].index_hdr, "GPU%d_%s", i, COMPUTECAP);
	    sprintf(gpu_mod_info[tmp+1].index_hdr, "GPU%d_%s", i, COMPUTEMODE);
    	sprintf(gpu_mod_info[tmp+2].index_hdr, "GPU%d_%s", i, KERNELEXETIMEOUTENABLE);
    	sprintf(gpu_mod_info[tmp+3].index_hdr, "GPU%d_%s", i, CONKERNELS);
    	sprintf(gpu_mod_info[tmp+4].index_hdr, "GPU%d_%s", i, STREAMPRIOR);
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
     
      snprintf (mod->info[tmp+0].index_data, LEN_32, "%d.%d", devInfo->major, devInfo->minor);
      snprintf (mod->info[tmp+1].index_data, LEN_32, "%s", (devInfo->core.computeMode == CZComputeModeDefault)? "Default":
		(devInfo->core.computeMode == CZComputeModeExclusive)? "Exclusive":
		(devInfo->core.computeMode == CZComputeModeProhibited)? "Prohibited":
		"Unknown");
      snprintf (mod->info[tmp+2].index_data, LEN_32, "%s", devInfo->core.kernelExecTimeoutEnabled? "Yes": "No");
      snprintf (mod->info[tmp+3].index_data, LEN_32, "%s", devInfo->core.concurrentKernels? "Yes": "No");
      snprintf (mod->info[tmp+4].index_data, LEN_32, "%s", devInfo->core.streamPrioritiesSupported? "Yes": "No");
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
