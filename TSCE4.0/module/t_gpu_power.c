#include <stdio.h>
#include "con_define.h"
#include "framework.h"
#include "common.h"
#include "t_gpu.h"

#define GPU_MODULE_COL_NUM 4


#define POWUSAGE "Power"
#define POWPSTATE "PowpState"
#define POWDEFAULTLIMIT "PowDfLim"
#define PCI "Pci"

struct PcieInfo {
//PCI info
  nvmlPciInfo_t pci;
};

struct GpuPower {
  uint32 PowUsage;
  //get power state
  nvmlPstates_t PowpState;
  //get power defaule limit
  uint32 PowdefaultLimit;
};

struct GpuDevice {

  nvmlReturn_t result;
  uint32 NumDevice;
  nvmlDevice_t Device[MAX_GPU_SUPPORT];

  struct GpuPower GPUPower[MAX_GPU_SUPPORT];
  struct PcieInfo Pcie[MAX_GPU_SUPPORT];
};

struct GpuDevice gpudevice;

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

//GPU power
uint32 getDevicePower() {
  uint32 i;
  for (i = 0; i < gpudevice.NumDevice; i++) {
    gpudevice.result = nvmlDeviceGetPowerUsage(gpudevice.Device[i], &(gpudevice.GPUPower[i].PowUsage));
    if (NVML_SUCCESS != gpudevice.result) {
	gpudevice.GPUPower[i].PowUsage = ERR;
	printf("ERROR: Failed to get power of device: %s\n", nvmlErrorString(gpudevice.result));
//	return GPU_PROPERTY_GET_FAILER;
    }

    gpudevice.result = nvmlDeviceGetPowerState(gpudevice.Device[i], &(gpudevice.GPUPower[i].PowpState));
    if (NVML_SUCCESS != gpudevice.result) {
	gpudevice.GPUPower[i].PowpState = ERR;
	printf("ERROR: Failed to get Power state of device: %s\n", nvmlErrorString(gpudevice.result));
//	return GPU_PROPERTY_GET_FAILER;
    }

    gpudevice.result = nvmlDeviceGetPowerManagementDefaultLimit(gpudevice.Device[i], &(gpudevice.GPUPower[i].PowdefaultLimit));
    if (NVML_SUCCESS != gpudevice.result) {
	gpudevice.GPUPower[i].PowdefaultLimit = ERR;
	printf("ERROR: Failed to get Power ManagementDefaultLimt  of device: %s\n", nvmlErrorString(gpudevice.result));
//	return GPU_PROPERTY_GET_FAILER;
    }
  }
  return GPU_PROPERTY_GET_SUCCESS;
}

/* pci info*/
uint32 getDevicePciInfo() {
    uint32 i;
    for (i = 0; i < gpudevice.NumDevice; i++) {
	gpudevice.result = nvmlDeviceGetPciInfo(gpudevice.Device[i], &(gpudevice.Pcie[i].pci));
	if (NVML_SUCCESS != gpudevice.result) {
		strcpy(gpudevice.Pcie[i].pci.busId, "Err");
		printf("ERROR: Failed to get PciInfo of device: %s\n", nvmlErrorString(gpudevice.result));
//		return GPU_PROPERTY_GET_FAILER;
      }
    }
  return GPU_PROPERTY_GET_SUCCESS;
}

static uint32 gpuDeviceInfoRead(struct module *mod) {
  uint32 ReturnStat = 0;
  uint32 i;
  uint32 tmp;

//GPU power
  ReturnStat = getDevicePower();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
    for ( i = 0; i < gpudevice.NumDevice; i++) {
      tmp = i * GPU_MODULE_COL_NUM; 
      snprintf (mod->info[tmp+0].index_data, LEN_32, "%.2f", gpudevice.GPUPower[i].PowUsage / 1000.0);
      snprintf (mod->info[tmp+1].index_data, LEN_32, "%d", gpudevice.GPUPower[i].PowpState);
      snprintf (mod->info[tmp+2].index_data, LEN_32, "%.2f", gpudevice.GPUPower[i].PowdefaultLimit/1000.0);
    }
  else
    return GPU_DEVICE_INFO_READ_FAILER;

  //PCIe
  ReturnStat = getDevicePciInfo();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
    for ( i = 0; i < gpudevice.NumDevice; i++) {
      tmp = i * GPU_MODULE_COL_NUM; 
      snprintf (mod->info[tmp+3].index_data, LEN_32, "%s", gpudevice.Pcie[i].pci.busId);
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
    	sprintf(gpu_mod_info[tmp+0].index_hdr, "GPU%d_%s", i, POWUSAGE);
    	sprintf(gpu_mod_info[tmp+1].index_hdr, "GPU%d_%s", i, POWPSTATE);
    	sprintf(gpu_mod_info[tmp+2].index_hdr, "GPU%d_%s", i, POWDEFAULTLIMIT);
    	sprintf(gpu_mod_info[tmp+3].index_hdr, "GPU%d_%s", i, PCI);
    }
    return 1;
}

void gpu_power_start()
{
	//NULL
}

void gpu_power_read(struct module *mod)
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
     					   	gpu_total_col_num, gpu_power_start, gpu_power_read);
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
