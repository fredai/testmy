#include <stdio.h>
#include "con_define.h"
#include "framework.h"
#include "common.h"
#include "t_gpu.h"

#define GPU_MODULE_COL_NUM 5


#define ECCMODCURRENT "EccModcurrent"
#define ECCMODPENDING "EccModpending"
#define ECCERRORTYPE "EccErrorType"
#define ECCCOUNTERTYPE "EccCounterType"
#define ECCCOUNTS "EccCounts"

struct GpuECC {
  //current and pending ECC modes
  nvmlEnableState_t EccModcurrent;
  nvmlEnableState_t EccModpending;
  //total ECC error counts for the device.
  nvmlMemoryErrorType_t EccErrorType;
  nvmlEccCounterType_t EccCounterType;
  unsigned long long  EccCounts;
};

static struct GpuDevice {

  nvmlReturn_t result;
  uint32 NumDevice;
  nvmlDevice_t Device[MAX_GPU_SUPPORT];

  struct GpuECC GPUEcc[MAX_GPU_SUPPORT];
};

static struct GpuDevice gpudevice;

static int CheckGPU = 0;
static int gpu_total_col_num = 0;
static struct mod_info gpu_mod_info[GPU_MODULE_COL_NUM * MAX_GPU_SUPPORT];



static uint32 gpuDeviceStart() {
  //Initialize NVML library
  gpudevice.result = nvmlInit();
  uint32 i;
  if (NVML_SUCCESS != gpudevice.result) { 
    printf("ERROR: Failed to initialize NVML: %s\n", nvmlErrorString(gpudevice.result));
    return GPU_UNABLE_INIT_NVML;
  }

  //Get the actrual number of GPU devices in a host
  gpudevice.result = nvmlDeviceGetCount(&(gpudevice.NumDevice));
  if (NVML_SUCCESS != gpudevice.result) {
    printf("ERROR: Failed to get the number of GPU devices: %s\n", nvmlErrorString(gpudevice.result));
    return GPU_UNABLE_GETNUMDEVICE;
  }
  
  //Get each of the GPU device's index
  for (i = 0; i < gpudevice.NumDevice; i++) {
    gpudevice.result = nvmlDeviceGetHandleByIndex(i, &(gpudevice.Device[i]));
    if (NVML_SUCCESS != gpudevice.result) { 
        printf("ERROR: Failed to get handle for GPU device: %s\n", nvmlErrorString(gpudevice.result));
    
      gpudevice.result = nvmlShutdown();
      if (NVML_SUCCESS != gpudevice.result) {
        printf("ERROR: Failed to shutdown NVML: %s\n",nvmlErrorString(gpudevice.result));
        return GPU_UNABLE_SHUTDOWN_NVML;
      }
      return GPU_UNABLE_GETDEVICEINDEX;
    }
  }
  return GPU_SUCCESS_START;
}

/* ECC management  */
uint32 getDeviceECCInfo() {
    uint32 i;
    for (i = 0; i < gpudevice.NumDevice; i++) {
        gpudevice.result = nvmlDeviceGetEccMode(gpudevice.Device[i], &(gpudevice.GPUEcc[i].EccModcurrent), &(gpudevice.GPUEcc[i].EccModpending));
        if (NVML_SUCCESS != gpudevice.result) {
	  gpudevice.GPUEcc[i].EccModcurrent = ERR;
	  gpudevice.GPUEcc[i].EccModpending = ERR;
          printf("ERROR: Failed to get Ecc Mode of device: %s\n", nvmlErrorString(gpudevice.result));
//        return GPU_PROPERTY_GET_FAILER;
      }

        gpudevice.result = nvmlDeviceGetTotalEccErrors(gpudevice.Device[i], gpudevice.GPUEcc[i].EccErrorType, gpudevice.GPUEcc[i].EccCounterType, &(gpudevice.GPUEcc[i].EccCounts));
        if (NVML_SUCCESS != gpudevice.result) {
	  gpudevice.GPUEcc[i].EccErrorType = ERR;
	  gpudevice.GPUEcc[i].EccCounterType = ERR;
	  gpudevice.GPUEcc[i].EccCounts = ERR;
          printf("ERROR: Failed to get Ecc Mode of device: %s\n", nvmlErrorString(gpudevice.result));
//        return GPU_PROPERTY_GET_FAILER;
      }

    }
    return GPU_PROPERTY_GET_SUCCESS;
}

static uint32 gpuDeviceInfoRead(struct module *mod) {
  uint32 ReturnStat = 0;
  uint32 i;
  uint32 tmp;

//ECC
  ReturnStat = getDeviceECCInfo();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
    for ( i = 0; i < gpudevice.NumDevice; i++) {
      tmp = i * GPU_MODULE_COL_NUM; 
      snprintf (mod->info[tmp+0].index_data, LEN_32, "%d", gpudevice.GPUEcc[i].EccModcurrent);
      snprintf (mod->info[tmp+1].index_data, LEN_32, "%d", gpudevice.GPUEcc[i].EccModpending);
      snprintf (mod->info[tmp+2].index_data, LEN_32, "%d", gpudevice.GPUEcc[i].EccErrorType);
      snprintf (mod->info[tmp+3].index_data, LEN_32, "%d", gpudevice.GPUEcc[i].EccCounterType);
      snprintf (mod->info[tmp+4].index_data, LEN_32, "%lld", gpudevice.GPUEcc[i].EccCounts);
    }
  else
    return GPU_DEVICE_INFO_READ_FAILER;

  return GPU_DEVICE_INFO_READ_SUCCESS;
}

static uint32 gpuDeviceStop() {
  
  gpudevice.result = nvmlShutdown();
  if (NVML_SUCCESS != gpudevice.result) {
  	printf("ERROR: Failed to shutdown NVML: %s\n",nvmlErrorString(gpudevice.result));
  	return GPU_UNABLE_SHUTDOWN_NVML;
  }
    return GPU_SUCCESS_STOP;
}


static int get_number_gpu()
{
    int i;
    int tmp;

    memset(gpu_mod_info, 0, sizeof(gpu_mod_info));
    if (GPU_SUCCESS_START == gpuDeviceStart())
	     CheckGPU = 1;
    else
	return -1;

    gpu_total_col_num = GPU_MODULE_COL_NUM * gpudevice.NumDevice;

    for ( i = 0; i < gpudevice.NumDevice; i++) {
	    tmp = i * GPU_MODULE_COL_NUM;
    	sprintf(gpu_mod_info[tmp+0].index_hdr, "GPU%d_%s", i, ECCMODCURRENT);
    	sprintf(gpu_mod_info[tmp+1].index_hdr, "GPU%d_%s", i, ECCMODPENDING);
    	sprintf(gpu_mod_info[tmp+2].index_hdr, "GPU%d_%s", i, ECCERRORTYPE);
    	sprintf(gpu_mod_info[tmp+3].index_hdr, "GPU%d_%s", i, ECCCOUNTERTYPE);
    	sprintf(gpu_mod_info[tmp+4].index_hdr, "GPU%d_%s", i, ECCCOUNTS);

    }
    return 1;
}

void gpu_ecc_start()
{
	//NULL
}

void gpu_ecc_read(struct module *mod)
{
	if (CheckGPU)
		if(GPU_DEVICE_INFO_READ_SUCCESS != gpuDeviceInfoRead(mod))
			CheckGPU = 0;
//	gpuDeviceStop();
	return;
}
         
                      
int
mod_register(struct module* mod)
{
    assert(mod != NULL);


//	gpudevice_fun();
	if (-1 == get_number_gpu()) {
            return MODULE_FLAG_NOT_USEABLE;
	}

        // TODO: add decide module is usealbe in current HW and SW environment 
        register_module_fields(mod, gpu_mod_info, \
     					   	gpu_total_col_num, gpu_ecc_start, gpu_ecc_read);
	return 0;
}


/*
main ()
{
        gpu_base_start();
        while(1)
        {
                gpu_base_read();
                sleep (1);
        }

  
}
*/
