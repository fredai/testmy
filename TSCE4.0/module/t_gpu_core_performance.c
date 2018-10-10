#include <stdio.h>
#include "con_define.h"
#include "framework.h"
#include "common.h"
#include "cudainfo.h"
#define GPU_MODULE_COL_NUM  5
#define MAX_GPU_SUPPORT     8

#define SINGLEFLOAT "Single-precision-Float"
#define DOUBLEFLOAT "Double-precision-Float"
#define SIXBITINTEGER "64-bit-Integer"
#define THREEBITINTEGER "32-bit-Integer"
#define TWENBITINTEGER "24-bit-Integer"
#define EBITINTEGER "8-bit-Integer"



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
    	sprintf(gpu_mod_info[tmp+0].index_hdr, "GPU%d_%s", i, SINGLEFLOAT);
    	sprintf(gpu_mod_info[tmp+1].index_hdr, "GPU%d_%s", i, DOUBLEFLOAT);
    	sprintf(gpu_mod_info[tmp+2].index_hdr, "GPU%d_%s", i, SIXBITINTEGER);
    	sprintf(gpu_mod_info[tmp+3].index_hdr, "GPU%d_%s", i, THREEBITINTEGER);
    	sprintf(gpu_mod_info[tmp+4].index_hdr, "GPU%d_%s", i, TWENBITINTEGER);
    	//sprintf(gpu_mod_info[tmp+5].index_hdr, "GPU%d_%s", i, EBITINTEGER);
    }
    return 0;
}

void gpu_rt_start()
{
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
	}
}

void gpu_rt_read(struct module *mod)
{
	uint32 i;
	uint32 tmp;
    for ( i = 0; i < numDev; i++)
	{
	  if (CZCudaCalcDevicePerformance(&devInfo[i]) < 0)
	  {
	  	continue;
	  }

	  tmp = i * GPU_MODULE_COL_NUM;
      snprintf (mod->info[tmp+0].index_data, LEN_32, "%.3f", devInfo[i].perf.calcFloat/1024/1024);
      snprintf (mod->info[tmp+1].index_data, LEN_32, "%.3f", devInfo[i].perf.calcDouble/1024/1024);
      snprintf (mod->info[tmp+2].index_data, LEN_32, "%.3f", devInfo[i].perf.calcInteger64/1024/1024);
      snprintf (mod->info[tmp+3].index_data, LEN_32, "%.3f", devInfo[i].perf.calcInteger32/1024/1024);
      snprintf (mod->info[tmp+4].index_data, LEN_32, "%.3f", devInfo[i].perf.calcInteger24/1024/1024);
      //snprintf (mod->info[tmp+5].index_data, LEN_32, "%.3f", devInfo[i].perf.calcInteger8/1024/1024);
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
