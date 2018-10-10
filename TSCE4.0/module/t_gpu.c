/*Inspur (Beijing) Electronic Information Industry Co., Ltd.
  
  Written by LiuYu
  
  File: t_gpu.cpp
  Version: V4.0.1
  Update: 2016-04-19
  
  Class gpudevice is used for getting GPU 
  informaction from gpu devices by using NVML. By using this 
  object one can get GPU utlization rate, SM clock, device memory 
  clock, device name, device memory total/used/free, and device 
  power useage.
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "t_gpu.h"

#include "con_define.h"
#include "framework.h"

#define GPU_MODULE_COL_NUM 4
#define GPUUTIL "Util"
#define GPUMEM "MEM"
#define GPUTEMP "TEMP"
#define GPUPOW "POW"
static int gpu_total_col_num = 0;

int CheckGPU = 0;

static struct mod_info gpu_mod_info[GPU_MODULE_COL_NUM * MAX_GPU_SUPPORT];

void gpudevice_fun() {
    uint32 i;
    gpudevice.result = 0;
    gpudevice.NumDevice  = 0;
  for (i = 0; i < MAX_GPU_SUPPORT; i++) {
    gpudevice.GPUSMClock[i] = 0;
    gpudevice.GPUMemClock[i] = 0;
    gpudevice.GPUPower[i] = 0;
    gpudevice.GPUTemperature[i] = 0;
    gpudevice.GPURate[i].gpu = 0;
    gpudevice.GPURate[i].memory = 0;
    gpudevice.GPUMemory[i].total = 0;
    gpudevice.GPUMemory[i].used = 0;
    gpudevice.GPUMemory[i].free = 0;
    strcpy(gpudevice.DeviceName[i], "NULL");
  }
}



uint32 gpuDeviceStart() {
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

//GPU temperature
uint32 getDeviceTemperature() {
  uint32 i;
  for (i = 0; i < gpudevice.NumDevice; i++) {
    gpudevice.result = nvmlDeviceGetTemperature(gpudevice.Device[i], NVML_TEMPERATURE_GPU, &(gpudevice.GPUTemperature[i]));
    if (NVML_SUCCESS != gpudevice.result) {
	printf("ERROR: Failed to get temperature of device: %s\n", nvmlErrorString(gpudevice.result));
      return GPU_PROPERTY_GET_FAILER;
    }
  }
  
  return GPU_PROPERTY_GET_SUCCESS;
}

//GPU utilizatioon rate
uint32 getDeviceUtilizationRate() {
  uint32 i;
  for (i = 0; i < gpudevice.NumDevice; i++) {
    gpudevice.result = nvmlDeviceGetUtilizationRates(gpudevice.Device[i], &(gpudevice.GPURate[i]));
    if (NVML_SUCCESS != gpudevice.result) {
	printf("ERROR: Failed to get utilization rate of device: %s\n", nvmlErrorString(gpudevice.result));
      return GPU_PROPERTY_GET_FAILER;
    }
  }

  return GPU_PROPERTY_GET_SUCCESS;
}

//GPU power
uint32 getDevicePower() {
  uint32 i;
  for (i = 0; i < gpudevice.NumDevice; i++) {
    gpudevice.result = nvmlDeviceGetPowerUsage(gpudevice.Device[i], &(gpudevice.GPUPower[i]));
    if (NVML_SUCCESS != gpudevice.result) {
	printf("ERROR: Failed to get power of device: %s\n", nvmlErrorString(gpudevice.result));
      return GPU_PROPERTY_GET_FAILER;
    }
  }

  return GPU_PROPERTY_GET_SUCCESS;
}

//Device name
uint32 getDeviceName() {
  uint32 i;
  for (i = 0; i < gpudevice.NumDevice; i++) {
    gpudevice.result = nvmlDeviceGetName(gpudevice.Device[i], gpudevice.DeviceName[i], 64);

    if (NVML_SUCCESS != gpudevice.result) {
	printf("ERROR: Failed to get the name of device: %s\n", nvmlErrorString(gpudevice.result));
      return GPU_PROPERTY_GET_FAILER;
    }
  }

  return GPU_PROPERTY_GET_SUCCESS;
}

//Device memory
uint32 getDeviceMemoryInfo() {
  uint32 i;
  for (i = 0; i < gpudevice.NumDevice; i++) {
    gpudevice.result = nvmlDeviceGetMemoryInfo(gpudevice.Device[i], &(gpudevice.GPUMemory[i]));
    if (NVML_SUCCESS != gpudevice.result) {
	printf("ERROR: Failed to get memory info of device: %s\n", nvmlErrorString(gpudevice.result));
      return GPU_PROPERTY_GET_FAILER;
    }
  }

  return GPU_PROPERTY_GET_SUCCESS;
}

uint32 getDeviceClock() {
  uint32 i;
  for (i = 0; i < gpudevice.NumDevice; i++) {
    //SM clock
    gpudevice.result = nvmlDeviceGetClockInfo(gpudevice.Device[i], NVML_CLOCK_SM , &(gpudevice.GPUSMClock[i]));
    if (NVML_SUCCESS != gpudevice.result) {
	printf("ERROR: Failed to get SM clock of device: %s\n", nvmlErrorString(gpudevice.result));
      return GPU_PROPERTY_GET_FAILER;
    }
    
    //Mem clock
    gpudevice.result = nvmlDeviceGetClockInfo(gpudevice.Device[i], NVML_CLOCK_MEM, &(gpudevice.GPUMemClock[i]));
    if (NVML_SUCCESS != gpudevice.result) {
	printf("ERROR: Failed to get mem clock of device: %s\n", nvmlErrorString(gpudevice.result));
      return GPU_PROPERTY_GET_FAILER;
    }
  }

  return GPU_PROPERTY_GET_SUCCESS;
}

uint32 gpuDeviceInfoRead(struct module *mod) {
  uint32 ReturnStat = 0;
  uint32 i;
  uint32 tmp;
  for (i = 0; i < MAX_GPU_SUPPORT; i++) {
    strcpy(gpu[i].DeviceName, "NULL");
    gpu[i].GPURate = 0.0;
    gpu[i].GPUMemRate = 0.0;
    gpu[i].GPUTotalMem = 0.0;
    gpu[i].GPUUsedMem = 0.0;
    gpu[i].GPUFreeMem = 0.0;
    gpu[i].GPUSMClock = 0.0;
    gpu[i].GPUMemClock = 0.0;
    gpu[i].GPUPower = 0.0;
    gpu[i].GPUTemperature = 0.0;
  }
  
  //Device name
  ReturnStat = getDeviceName();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
    for (i = 0; i < gpudevice.NumDevice; i++)
      strcpy(gpu[i].DeviceName, gpudevice.DeviceName[i]);
  else
    return GPU_DEVICE_INFO_READ_FAILER;
  
  //Utilization rate
  ReturnStat = getDeviceUtilizationRate();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
    for (i = 0; i < gpudevice.NumDevice; i++) {
      tmp = i * GPU_MODULE_COL_NUM;
      gpu[i].GPURate = gpudevice.GPURate[i].gpu;
      gpu[i].GPUMemRate = gpudevice.GPURate[i].memory;
      snprintf (mod->info[tmp+0].index_data, LEN_32, "%.2f", gpu[i].GPURate);
      snprintf (mod->info[tmp+1].index_data, LEN_32, "%.2f", gpu[i].GPUMemRate);
    }
  else
    return GPU_DEVICE_INFO_READ_FAILER;
  
  //Memory useage
  ReturnStat = getDeviceMemoryInfo();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
    for (i = 0; i < gpudevice.NumDevice; i++) {
      gpu[i].GPUTotalMem = gpudevice.GPUMemory[i].total / 1024.0 / 1024.0;
      gpu[i].GPUUsedMem = gpudevice.GPUMemory[i].used / 1024.0 / 1024.0;
      gpu[i].GPUFreeMem = gpudevice.GPUMemory[i].free / 1024.0 / 1024.0;
    }
  else
    return GPU_DEVICE_INFO_READ_FAILER;
  
  //GPU clock
  ReturnStat = getDeviceClock();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
    for ( i = 0; i < gpudevice.NumDevice; i++) {
      gpu[i].GPUSMClock = gpudevice.GPUSMClock[i];
      gpu[i].GPUMemClock = gpudevice.GPUMemClock[i];
    }
  else
    return GPU_DEVICE_INFO_READ_FAILER;
  
  //GPU power
  ReturnStat = getDevicePower();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
    for ( i = 0; i < gpudevice.NumDevice; i++) {
      tmp = i * GPU_MODULE_COL_NUM;
      gpu[i].GPUPower = gpudevice.GPUPower[i] / 1000.0;
      snprintf (mod->info[tmp+3].index_data, LEN_32, "%.2f", gpu[i].GPUPower);
    }
	
  else
    return GPU_DEVICE_INFO_READ_FAILER;
  
  //GPU temperature
  ReturnStat = getDeviceTemperature();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
    for ( i = 0; i < gpudevice.NumDevice; i++) {
      tmp = i * GPU_MODULE_COL_NUM;
      gpu[i].GPUTemperature = gpudevice.GPUTemperature[i];
      snprintf (mod->info[tmp+2].index_data, LEN_32, "%.2f", gpu[i].GPUTemperature);
    }
  else
    return GPU_DEVICE_INFO_READ_FAILER;

  return GPU_DEVICE_INFO_READ_SUCCESS;
}

uint32 gpuDeviceStop() {
  
  gpudevice.result = nvmlShutdown();
  if (NVML_SUCCESS != gpudevice.result) {
	printf("ERROR: Failed to shutdown NVML: %s\n",nvmlErrorString(gpudevice.result));
	return GPU_UNABLE_SHUTDOWN_NVML;
  }
  
  return GPU_SUCCESS_STOP;
}


int get_number_gpu()
{
    int i;
    int tmp;

    memset(gpu_mod_info, 0, sizeof(gpu_mod_info));
    if (GPU_SUCCESS_START == gpuDeviceStart())
	CheckGPU = 1;

    gpu_total_col_num = GPU_MODULE_COL_NUM * gpudevice.NumDevice;
    for ( i = 0; i < gpudevice.NumDevice; i++) {
	tmp = i * GPU_MODULE_COL_NUM;
    	sprintf(gpu_mod_info[tmp+0].index_hdr, "GPU%d_%s", i, GPUUTIL);
    	sprintf(gpu_mod_info[tmp+1].index_hdr, "GPU%d_%s", i, GPUMEM);
    	sprintf(gpu_mod_info[tmp+2].index_hdr, "GPU%d_%s", i, GPUTEMP);
    	sprintf(gpu_mod_info[tmp+3].index_hdr, "GPU%d_%s", i, GPUPOW);
    }

    return 1;
}

void gpu_read(struct module *mod)
{
	if (CheckGPU)
		if(GPU_DEVICE_INFO_READ_SUCCESS != gpuDeviceInfoRead(mod))
			CheckGPU = 0;
	gpuDeviceStop();
	return;
}

void gpu_start()
{
	//NULL
}

int
mod_register(struct module* mod)
{
    assert(mod != NULL);


	gpudevice_fun();
	if (-1 == get_number_gpu()) {
            return MODULE_FLAG_NOT_USEABLE;
	}

        // TODO: add decide module is usealbe in current HW and SW environment 
        register_module_fields(mod, gpu_mod_info, \
                                                  gpu_total_col_num, gpu_start, gpu_read);
        return 0;
}


/*
main ()
{
	gpu_start();
	gpu_read();
}
*/

