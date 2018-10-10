#include <stdio.h>
#include "con_define.h"
#include "framework.h"
#include "common.h"
#include "t_gpu.h"

#define GPU_MODULE_COL_NUM 7

#define GPUNAME "Name"
#define GPUTEMP "GPUTemp"
#define URGPU "GPUUtil"
#define URMEM "MemUsage"
#define MEMTOTAL "MemTotal"
#define MEMFREE "MemFree"
#define MEMUSED "MemUsed"
#define GPUNUM "gpu_number"

static struct GpuDevice {

  nvmlReturn_t result;
  uint32 NumDevice;
  nvmlDevice_t Device[MAX_GPU_SUPPORT];
  char DeviceName[MAX_GPU_SUPPORT][64];
//gpu basic info
  uint32 GPUTemperature[MAX_GPU_SUPPORT];  
  nvmlUtilization_t GPURate[MAX_GPU_SUPPORT];
  nvmlMemory_t GPUMemory[MAX_GPU_SUPPORT];
};

static struct GpuDevice gpudevice;

static int CheckGPU = 0;
static int gpu_total_col_num = 0;
static struct mod_info gpu_mod_info[GPU_MODULE_COL_NUM * MAX_GPU_SUPPORT];

/*
void gpudevice_fun() {
    gpudevice.result = 0;
    gpudevice.NumDevice  = 0;
    uint32 i;
  for (i = 0; i < MAX_GPU_SUPPORT; i++) {
    gpudevice.GPUClock[i].GPUSMClock = 0;
    gpudevice.GPUClock[i].GPUMemClock = 0;
    gpudevice.GPUPower[i].PowUsage = 0;
    gpudevice.GPUTemperature[i] = 0;
    gpudevice.GPURate[i].gpu = 0;
    gpudevice.GPURate[i].memory = 0;
    gpudevice.GPUMemory[i].total = 0;
    gpudevice.GPUMemory[i].used = 0;
    gpudevice.GPUMemory[i].free = 0;
    strcpy(gpudevice.DeviceName[i], "NULL");
  }
}
*/


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

uint32 getDeviceName() {
  uint32 i;
  for (i = 0; i < gpudevice.NumDevice; i++) {
    gpudevice.result = nvmlDeviceGetName(gpudevice.Device[i], gpudevice.DeviceName[i], 64);
    if (NVML_SUCCESS != gpudevice.result) {
      strcpy (gpudevice.DeviceName[i], "Err");
      printf("ERROR: Failed to get the name of device: %s\n", nvmlErrorString(gpudevice.result));
//      return GPU_PROPERTY_GET_FAILER;
    }
  }
    return GPU_PROPERTY_GET_SUCCESS;
}

//GPU temperature
uint32 getDeviceTemperature() {
  uint32 i;
  for (i = 0; i < gpudevice.NumDevice; i++) {
    gpudevice.result = nvmlDeviceGetTemperature(gpudevice.Device[i], NVML_TEMPERATURE_GPU, &(gpudevice.GPUTemperature[i]));
    if (NVML_SUCCESS != gpudevice.result) {
	gpudevice.GPUTemperature[i] = ERR;
  	printf("ERROR: Failed to get temperature of device: %s\n", nvmlErrorString(gpudevice.result));
//	return GPU_PROPERTY_GET_FAILER;
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
      gpudevice.GPUMemory[i].free = ERR;
      gpudevice.GPUMemory[i].used = ERR;
      gpudevice.GPUMemory[i].total = ERR;
      printf("ERROR: Failed to get memory info of device: %s\n", nvmlErrorString(gpudevice.result));
//    return GPU_PROPERTY_GET_FAILER;
    }
  }
  return GPU_PROPERTY_GET_SUCCESS;
}


//GPU utilizatioon rate
uint32 getDeviceURInfo() {

  uint32 i;
  for (i = 0; i < gpudevice.NumDevice; i++) {
    gpudevice.result = nvmlDeviceGetUtilizationRates(gpudevice.Device[i], &(gpudevice.GPURate[i]));
    if (NVML_SUCCESS != gpudevice.result) {
      gpudevice.GPURate[i].gpu = ERR;
      gpudevice.GPURate[i].memory = ERR;
      printf("ERROR: Failed to get utilization rate of device: %s\n", nvmlErrorString(gpudevice.result));
//    return GPU_PROPERTY_GET_FAILER;
    }
  }
    return GPU_PROPERTY_GET_SUCCESS;
}

static uint32 gpuDeviceInfoRead(struct module *mod) {
  uint32 ReturnStat = 0;
  uint32 i;
  uint32 tmp;

//Device Name
  ReturnStat = getDeviceName();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
  for ( i = 0; i < gpudevice.NumDevice; i++) {
    tmp = i * GPU_MODULE_COL_NUM; 
    snprintf (mod->info[tmp+0].index_data, LEN_32, "%s", gpudevice.DeviceName[i]);
  }
  else
    return GPU_DEVICE_INFO_READ_FAILER;

//GPU temperature
  ReturnStat = getDeviceTemperature();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
    for ( i = 0; i < gpudevice.NumDevice; i++) {
      tmp = i * GPU_MODULE_COL_NUM; 
      snprintf (mod->info[tmp+1].index_data, LEN_32, "%d", gpudevice.GPUTemperature[i]);
//    printf("gpu temperature = %.2f\n", gpudevice.GPUTemperature[i]);
    }
  else
    return GPU_DEVICE_INFO_READ_FAILER;

// GPU utilizatioon rate
  ReturnStat =  getDeviceURInfo();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
  for ( i = 0; i < gpudevice.NumDevice; i++) {
    tmp = i * GPU_MODULE_COL_NUM; 
    snprintf (mod->info[tmp+2].index_data, LEN_32, "%d", gpudevice.GPURate[i].gpu);
//    snprintf (mod->info[tmp+3].index_data, LEN_32, "%.2f", gpudevice.GPURate[i].memory);
  }
  else
    return GPU_DEVICE_INFO_READ_FAILER;
    
//Memory useage
  ReturnStat = getDeviceMemoryInfo();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
    for (i = 0; i < gpudevice.NumDevice; i++) {
      tmp = i * GPU_MODULE_COL_NUM;	
      snprintf (mod->info[tmp+4].index_data, LEN_32, "%.2f", gpudevice.GPUMemory[i].total / 1024.0 / 1024.0);
      snprintf (mod->info[tmp+5].index_data, LEN_32, "%.2f", gpudevice.GPUMemory[i].free / 1024.0 / 1024.0);
      snprintf (mod->info[tmp+6].index_data, LEN_32, "%.2f", gpudevice.GPUMemory[i].used / 1024.0 / 1024.0);
      snprintf (mod->info[tmp+3].index_data, LEN_32, "%.2f", gpudevice.GPUMemory[i].used*100.0/gpudevice.GPUMemory[i].total);
    }
  else
    return GPU_DEVICE_INFO_READ_FAILER;
    snprintf (mod->info[tmp+7].index_data, LEN_32, "%d", gpudevice.NumDevice);

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

    gpu_total_col_num = GPU_MODULE_COL_NUM * gpudevice.NumDevice + 1;

//    	sprintf(gpu_mod_info[0].index_hdr, "%d", gpudevice.NumDevice);
    for ( i = 0; i < gpudevice.NumDevice; i++) {
	tmp = i * GPU_MODULE_COL_NUM;
    	sprintf(gpu_mod_info[tmp+0].index_hdr, "GPU%d_%s", i, GPUNAME);
    	sprintf(gpu_mod_info[tmp+1].index_hdr, "GPU%d_%s", i, GPUTEMP);
    	sprintf(gpu_mod_info[tmp+2].index_hdr, "GPU%d_%s", i, URGPU);
    	sprintf(gpu_mod_info[tmp+3].index_hdr, "GPU%d_%s", i, URMEM);
    	sprintf(gpu_mod_info[tmp+4].index_hdr, "GPU%d_%s", i, MEMTOTAL);
    	sprintf(gpu_mod_info[tmp+5].index_hdr, "GPU%d_%s", i, MEMFREE);
     	sprintf(gpu_mod_info[tmp+6].index_hdr, "GPU%d_%s", i, MEMUSED);
    }
    	sprintf(gpu_mod_info[tmp+7].index_hdr, "%s", GPUNUM);
    return 1;
}

void gpu_base1_start()
{
	//NULL
}

void gpu_base1_read(struct module *mod)
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
     					   	gpu_total_col_num, gpu_base1_start, gpu_base1_read);
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
