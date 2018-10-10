/*Inspur (Beijing) Electronic Information Industry Co., Ltd.
  
  Written by daizhenyu
  
  File: t_gpu.h
  Version: V4.0.0
  Update: 2017-02-20
  
  Head file of t_gpu.cpp which is used for getting GPU 
  informaction from the GPU devices by using NVML.  By using this
  object one can get GPU utlization rate, SM clock, device memory
  clock, device name, device memory total/used/free, and device
  power useage.
*/

#ifndef _T_GPU_H_
#define _T_GPU_H_

#include "nvml.h"

//typedef unsigned int uint32;
#define MAX_GPU_SUPPORT 8

#define GPU_UNABLE_INIT_NVML         1
#define GPU_UNABLE_GETNUMDEVICE      2
#define GPU_UNABLE_GETDEVICEINDEX    3
#define GPU_UNABLE_SHUTDOWN_NVML     4
#define GPU_SUCCESS_START            5
#define GPU_PROPERTY_GET_FAILER      6
#define GPU_DEVICE_INFO_READ_FAILER  7
#define GPU_DEVICE_INFO_READ_SUCCESS 8
#define GPU_PROPERTY_GET_SUCCESS     9
#define GPU_SUCCESS_STOP             10
#define ERR			     -1

//base
//#define GPUNUM "gpu_number"

#endif
