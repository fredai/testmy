#include <stdio.h>
#include "con_define.h"
#include "framework.h"
#include "common.h"
#include "t_gpu.h"

#define GPU_MODULE_COL_NUM 7

//Clock
#define GPUSMCLOCK "GPUSMClock"
#define GPUMEMCLOCK "GPUMemClock"
#define MAXSMCLOCK "MaxSMClock"
#define MAXMEMCLOCK "MaxMemClock"
#define APPCURSMCLOCK "APPCURSMCLOCK"
#define APPCURMEMCLOCK "APPCURMemCLOCK"
#define CLOCKSTHROTTLEREASONS "ThrottleInfo"

struct GpuClock {
//GPU clock
  uint32 GPUSMClock;
  uint32 GPUMemClock;
//GPUMax Clock
  uint32 MaxSMClock;
  uint32 MaxMemClock;
//App Current Clock
  uint32 AppCurSMClock;
  uint32 AppCurMemClock;
// GPU Throttle Reasons;
  unsigned long long ClocksThrottleReasons;
};

static struct GpuDevice {

  nvmlReturn_t result;
  uint32 NumDevice;
  nvmlDevice_t Device[MAX_GPU_SUPPORT];

  struct GpuClock GPUClock[MAX_GPU_SUPPORT];
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




/*get device clock*/
uint32 getDeviceClock() {
  uint32 i;
  for (i = 0; i < gpudevice.NumDevice; i++) {
    //SM clock
    gpudevice.result = nvmlDeviceGetClockInfo(gpudevice.Device[i], NVML_CLOCK_SM , &(gpudevice.GPUClock[i].GPUSMClock));
    if (NVML_SUCCESS != gpudevice.result) {
	gpudevice.GPUClock[i].GPUSMClock = ERR;	
	printf("ERROR: Failed to get SM clock of device: %s\n", nvmlErrorString(gpudevice.result));
//	return GPU_PROPERTY_GET_FAILER;
    }
    
    //Mem clock
    gpudevice.result = nvmlDeviceGetClockInfo(gpudevice.Device[i], NVML_CLOCK_MEM, &(gpudevice.GPUClock[i].GPUMemClock));
    if (NVML_SUCCESS != gpudevice.result) {
	gpudevice.GPUClock[i].GPUMemClock = ERR;
	printf("ERROR: Failed to get mem clock of device: %s\n", nvmlErrorString(gpudevice.result));
//	return GPU_PROPERTY_GET_FAILER;
    }

    gpudevice.result = nvmlDeviceGetMaxClockInfo(gpudevice.Device[i], NVML_CLOCK_SM, &(gpudevice.GPUClock[i].MaxSMClock));
    if (NVML_SUCCESS != gpudevice.result) {
	gpudevice.GPUClock[i].MaxSMClock = ERR;
	printf("ERROR: Failed to get MaxSM clock of device: %s\n", nvmlErrorString(gpudevice.result));
//	return GPU_PROPERTY_GET_FAILER;
    }

    gpudevice.result = nvmlDeviceGetMaxClockInfo(gpudevice.Device[i], NVML_CLOCK_MEM, &(gpudevice.GPUClock[i].MaxMemClock));
    if (NVML_SUCCESS != gpudevice.result) {
	gpudevice.GPUClock[i].MaxMemClock = ERR;
	printf("ERROR: Failed to get MaxMEM clock of device: %s\n", nvmlErrorString(gpudevice.result));
//	return GPU_PROPERTY_GET_FAILER;
    }

    gpudevice.result = nvmlDeviceGetApplicationsClock(gpudevice.Device[i], NVML_CLOCK_SM, &(gpudevice.GPUClock[i].AppCurSMClock));
    if (NVML_SUCCESS != gpudevice.result) {
	gpudevice.GPUClock[i].AppCurSMClock = ERR;
	printf("ERROR: Failed to get APP SM Clock of device: %s\n", nvmlErrorString(gpudevice.result));
//	return GPU_PROPERTY_GET_FAILER;
    }

    gpudevice.result = nvmlDeviceGetApplicationsClock(gpudevice.Device[i], NVML_CLOCK_MEM, &(gpudevice.GPUClock[i].AppCurMemClock));
    if (NVML_SUCCESS != gpudevice.result) {
	gpudevice.GPUClock[i].AppCurMemClock = ERR;
	printf("ERROR: Failed to get APP MEM Clock of device: %s\n", nvmlErrorString(gpudevice.result));
//	return GPU_PROPERTY_GET_FAILER;
    }
    gpudevice.result = nvmlDeviceGetCurrentClocksThrottleReasons(gpudevice.Device[i], &(gpudevice.GPUClock[i].ClocksThrottleReasons));
    if (NVML_SUCCESS != gpudevice.result) {
	gpudevice.GPUClock[i].ClocksThrottleReasons = ERR;
	printf("ERROR: Failed to get Current clock throttle reasons of device: %s\n", nvmlErrorString(gpudevice.result));
//	return GPU_PROPERTY_GET_FAILER;
    }
  }
  return GPU_PROPERTY_GET_SUCCESS;
}


static uint32 gpuDeviceInfoRead(struct module *mod) {
  uint32 ReturnStat = 0;
  uint32 i;
  uint32 tmp;

//GPU clock
  ReturnStat = getDeviceClock();
  if (GPU_PROPERTY_GET_SUCCESS == ReturnStat)
    for ( i = 0; i < gpudevice.NumDevice; i++) {
  tmp = i * GPU_MODULE_COL_NUM; 
      snprintf (mod->info[tmp+0].index_data, LEN_32,"%d", gpudevice.GPUClock[i].GPUSMClock);
      snprintf (mod->info[tmp+1].index_data, LEN_32,"%d", gpudevice.GPUClock[i].GPUMemClock);
      snprintf (mod->info[tmp+2].index_data, LEN_32,"%d", gpudevice.GPUClock[i].MaxSMClock);
      snprintf (mod->info[tmp+3].index_data, LEN_32,"%d", gpudevice.GPUClock[i].MaxMemClock);
      snprintf (mod->info[tmp+4].index_data, LEN_32,"%d", gpudevice.GPUClock[i].AppCurSMClock);
      snprintf (mod->info[tmp+5].index_data, LEN_32,"%d", gpudevice.GPUClock[i].AppCurMemClock);
      snprintf (mod->info[tmp+6].index_data, LEN_32,"%lld", gpudevice.GPUClock[i].ClocksThrottleReasons);
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
    	sprintf(gpu_mod_info[tmp+0].index_hdr, "GPU%d_%s", i, GPUSMCLOCK);
    	sprintf(gpu_mod_info[tmp+1].index_hdr, "GPU%d_%s", i, GPUMEMCLOCK);
    	sprintf(gpu_mod_info[tmp+2].index_hdr, "GPU%d_%s", i, MAXSMCLOCK);
    	sprintf(gpu_mod_info[tmp+3].index_hdr, "GPU%d_%s", i, MAXMEMCLOCK);
    	sprintf(gpu_mod_info[tmp+4].index_hdr, "GPU%d_%s", i, APPCURSMCLOCK);
      sprintf(gpu_mod_info[tmp+5].index_hdr, "GPU%d_%s", i, APPCURMEMCLOCK);
      sprintf(gpu_mod_info[tmp+6].index_hdr, "GPU%d_%s", i, CLOCKSTHROTTLEREASONS);

    }
    return 1;
}

void gpu_clock_start()
{
	//NULL
}

void gpu_clock_read(struct module *mod)
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
     					   	gpu_total_col_num, gpu_clock_start, gpu_clock_read);
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
