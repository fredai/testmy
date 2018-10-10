#include <stdio.h>
#include "con_define.h"
#include "framework.h"
#include "common.h"
#include "cudainfo.h"
#define GPU_MODULE_COL_NUM  4
#define MAX_GPU_SUPPORT     8

#define TEXTALIGN "TextureAlign"
#define TEXT1DSIZE "Text1DSize"
#define TEXT2DSIZE "Text2DSize"
#define TEXT3DSIZE "Text3DSize"



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
      sprintf(gpu_mod_info[tmp+0].index_hdr, "GPU%d_%s", i, TEXTALIGN);
      sprintf(gpu_mod_info[tmp+1].index_hdr, "GPU%d_%s", i, TEXT1DSIZE);
      sprintf(gpu_mod_info[tmp+2].index_hdr, "GPU%d_%s", i, TEXT2DSIZE);
      sprintf(gpu_mod_info[tmp+3].index_hdr, "GPU%d_%s", i, TEXT3DSIZE);
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
      snprintf (mod->info[tmp+0].index_data, LEN_32, "%d", devInfo->mem.textureAlignment);
      snprintf (mod->info[tmp+1].index_data, LEN_32, "%d", devInfo->mem.texture1D[0]);
      snprintf (mod->info[tmp+2].index_data, LEN_32, "%dx%d", devInfo->mem.texture2D[0],
        devInfo->mem.texture2D[1]);
      snprintf (mod->info[tmp+3].index_data, LEN_32, "%dx%dx%d", devInfo->mem.texture3D[0],
        devInfo->mem.texture3D[1], devInfo->mem.texture3D[2]);
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
