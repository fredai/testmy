#include <stdio.h>
#include "con_define.h"
#include "framework.h"
#include "t_gpu.h"
#include "common.h"

#define GPU_MODULE_COL_NUM 7

#define FANSPEED "FanSpeed"
#define CUMODE "CuMode"
#define PSTATE "pState"
#define GOMCURRENT "GOMCurrent"
#define GOMPENDING "GOMPending"
#define PCIMAXWIDTH "PciMaxWidth"
#define PCICURWIDTH "PciCurWidth"

static struct GpuDevice {

  nvmlReturn_t result;
  uint32 NumDevice;
  nvmlDevice_t Device[MAX_GPU_SUPPORT];


  uint32 FanSpeed[MAX_GPU_SUPPORT];
  nvmlComputeMode_t CuMode[MAX_GPU_SUPPORT];
  nvmlPstates_t pState[MAX_GPU_SUPPORT];
//DeviceGetGpuOperationMode
  nvmlGpuOperationMode_t GOMCurrent[MAX_GPU_SUPPORT];
  nvmlGpuOperationMode_t GOMPending[MAX_GPU_SUPPORT];

//DeviceGetGpuLinkWidth
  unsigned int PciMaxLinkWidth[MAX_GPU_SUPPORT];
  unsigned int PciCurrLinkWidth[MAX_GPU_SUPPORT];

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

//Fan Speed
uint32 getDeviceFanSpeed() {
  uint32 i;
  for (i = 0; i < gpudevice.NumDevice; i++) {
    gpudevice.result = nvmlDeviceGetFanSpeed(gpudevice.Device[i], &(gpudevice.FanSpeed[i]));
    if (NVML_SUCCESS != gpudevice.result) {
      gpudevice.FanSpeed[i] = ERR;
    }
  }
  return GPU_PROPERTY_GET_SUCCESS;
}

//GPU Compute mode
uint32 getDeviceCuMode() {
  uint32 i;
  for (i = 0; i < gpudevice.NumDevice; i++) {
        gpudevice.result = nvmlDeviceGetComputeMode(gpudevice.Device[i], &(gpudevice.CuMode[i]));
    if (NVML_SUCCESS != gpudevice.result) {
	gpudevice.CuMode[i] = ERR;
        printf("ERROR: Failed to get Gompute Mode of device: %s\n", nvmlErrorString(gpudevice.result));
//      return GPU_PROPERTY_GET_FAILER;
    }
  }
  return GPU_PROPERTY_GET_SUCCESS;
}


//Device pState
uint32 getDevicepState() {
  uint32 i;
  for (i = 0; i < gpudevice.NumDevice; i++) {
    gpudevice.result = nvmlDeviceGetPerformanceState(gpudevice.Device[i],&(gpudevice.pState[i]));
    if (NVML_SUCCESS != gpudevice.result) {
	gpudevice.pState[i] = ERR;
        printf("ERROR: Failed to get Performance State of device: %s\n", nvmlErrorString(gpudevice.result));
//      return GPU_PROPERTY_GET_FAILER;
    }
  }
  return GPU_PROPERTY_GET_SUCCESS;
}


//get device operation mode
uint32 getDeviceGOM() {
  uint32 i;
  for (i = 0; i < gpudevice.NumDevice; i++) {
    gpudevice.result = nvmlDeviceGetGpuOperationMode(gpudevice.Device[i],&(gpudevice.GOMCurrent[i]), &(gpudevice.GOMPending[i]));
    if (NVML_SUCCESS != gpudevice.result) {
      gpudevice.GOMCurrent[i] = ERR;
      gpudevice.GOMPending[i] = ERR;
    }
  }
  return GPU_PROPERTY_GET_SUCCESS;
}

//get device PciMaxLinkWidth
uint32 getLinkWidth() {
  uint32 i;
  for (i = 0; i < gpudevice.NumDevice; i++) {
    gpudevice.result = nvmlDeviceGetMaxPcieLinkWidth (gpudevice.Device[i],&(gpudevice.PciMaxLinkWidth[i]));
    if (NVML_SUCCESS != gpudevice.result) {
      gpudevice.PciMaxLinkWidth[i] = ERR;
    }
    gpudevice.result = nvmlDeviceGetCurrPcieLinkWidth (gpudevice.Device[i],&(gpudevice.PciCurrLinkWidth[i]));
    if (NVML_SUCCESS != gpudevice.result) {
      gpudevice.PciCurrLinkWidth[i] = ERR;
    }
  }
  return GPU_PROPERTY_GET_SUCCESS;
}

static uint32 gpuDeviceInfoRead(struct module *mod) {
  uint32 ReturnStat = 0;
  uint32 i;
  uint32 tmp;

//Fan Speed
  ReturnStat = getDeviceFanSpeed();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
  for ( i = 0; i < gpudevice.NumDevice; i++) {
    tmp = i * GPU_MODULE_COL_NUM; 
    snprintf (mod->info[tmp+0].index_data, LEN_32, "%d", gpudevice.FanSpeed[i]);
  }
  else
    return GPU_DEVICE_INFO_READ_FAILER;

//GPU Compute mode
  ReturnStat = getDeviceCuMode();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
    for ( i = 0; i < gpudevice.NumDevice; i++) {
      tmp = i * GPU_MODULE_COL_NUM; 
      snprintf (mod->info[tmp+1].index_data, LEN_32, "%d", gpudevice.CuMode[i]);
    }
  else
    return GPU_DEVICE_INFO_READ_FAILER;

//Device pState
  ReturnStat =  getDevicepState();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
  for ( i = 0; i < gpudevice.NumDevice; i++) {
    tmp = i * GPU_MODULE_COL_NUM; 
    snprintf (mod->info[tmp+2].index_data, LEN_32, "%d", gpudevice.pState[i]);
  }
  else
    return GPU_DEVICE_INFO_READ_FAILER;
    
//get device operation mode
  ReturnStat = getDeviceGOM();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
    for (i = 0; i < gpudevice.NumDevice; i++) {
      tmp = i * GPU_MODULE_COL_NUM;	
        snprintf (mod->info[tmp+3].index_data, LEN_32, "%d", gpudevice.GOMCurrent[i]);
        snprintf (mod->info[tmp+4].index_data, LEN_32, "%d", gpudevice.GOMPending[i]);
    }
  else
    return GPU_DEVICE_INFO_READ_FAILER;

  ReturnStat = getLinkWidth();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
    for (i = 0; i < gpudevice.NumDevice; i++) {
      tmp = i * GPU_MODULE_COL_NUM;	
        snprintf (mod->info[tmp+5].index_data, LEN_32, "%d", gpudevice.PciMaxLinkWidth[i]);
        snprintf (mod->info[tmp+6].index_data, LEN_32, "%d", gpudevice.PciCurrLinkWidth[i]);
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
    	sprintf(gpu_mod_info[tmp+0].index_hdr, "GPU%d_%s", i, FANSPEED);
    	sprintf(gpu_mod_info[tmp+1].index_hdr, "GPU%d_%s", i, CUMODE);
    	sprintf(gpu_mod_info[tmp+2].index_hdr, "GPU%d_%s", i, PSTATE);
    	sprintf(gpu_mod_info[tmp+3].index_hdr, "GPU%d_%s", i, GOMCURRENT);
    	sprintf(gpu_mod_info[tmp+4].index_hdr, "GPU%d_%s", i, GOMPENDING);
    	sprintf(gpu_mod_info[tmp+5].index_hdr, "GPU%d_%s", i, PCIMAXWIDTH);
    	sprintf(gpu_mod_info[tmp+6].index_hdr, "GPU%d_%s", i, PCICURWIDTH);

    }
    return 1;
}

void gpu_base2_start()
{
	//NULL
}

void gpu_base2_read(struct module *mod)
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
     					   	gpu_total_col_num, gpu_base2_start, gpu_base2_read);
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
