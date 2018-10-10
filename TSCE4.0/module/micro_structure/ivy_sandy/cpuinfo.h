/*Inspur (Beijing) Electronic Information Industry Co., Ltd.
  
  Written by LiuYu
  
  File: cpuinfo.h
  Version: V1.0 beta1
  Update: 2013-02-07
  
  Head file of cpuinfo.cpp which is used for getting CPU 
  informaction from the host, such as, number of cores, 
  number of sockets, number of core general counters and so on.
*/

#ifndef _CPUINFO_H_
#define _CPUINFO_H_

//#include <string>
#include <string.h>
#include <semaphore.h>
#include "types.h"

#define GETCPUID(id, eax, ebx, ecx, edx) \
  __asm__ __volatile__ ("cpuid"	\
			: "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx) \
			: "a" (id));

sem_t IvySandy_pcie_sem_get;
sem_t IvySandy_pcie_sem_put;

typedef struct class_cpuinfo cpuinfo;
struct class_cpuinfo {

  //Cores here represent both logical core and physical core.
  uint32 NumCores;

  uint32 NumSockets;
  uint32 ThreadsPerCore;

  char CPUVendor[256];
  uint32 CPUFamily;
  uint32 CPUModel;

  //For CPU Performance Monitoring Units.
  uint32 NumCoreGeneralCounters;
  uint32 NumCoreFixedCounters;
  uint32 CoreGeneralCounterWidth;
  uint32 CoreFixedCounterWidth;
  uint32 PerformanceMonitorVersion;

  uint64 (*extractBits)(uint64 value, uint32 begin, uint32 end);

  void (*cpuinfo)(struct class_cpuinfo *th);
  void (*Ucpuinfo)(struct class_cpuinfo *th);

  void (*getCPUInfo)(struct class_cpuinfo *th);

  uint32 (*getNumCores)(struct class_cpuinfo *th);
  uint32 (*getNumSockets)(struct class_cpuinfo *th);
  uint32 (*getThreadsPerCore)(struct class_cpuinfo *th);
  char * (*getCPUVendor)(struct class_cpuinfo *th) ;
  uint32 (*getCPUFamily)(struct class_cpuinfo *th) ;
  uint32 (*getCPUModel)(struct class_cpuinfo *th) ;
  uint32 (*getNumCoreGeneralCounters)(struct class_cpuinfo *th) ;
  uint32 (*getNumCoreFixedCounters)(struct class_cpuinfo *th) ;
  uint32 (*getCoreGeneralCounterWidth)(struct class_cpuinfo *th) ;
  uint32 (*getCoreFixedCounterWidth)(struct class_cpuinfo *th) ;
  uint32 (*getPerformanceMonitorVersion)(struct class_cpuinfo *th) ;
  
};

void cpuinfo_cpuinfo(cpuinfo *th);
void cpuinfo_Ucpuinfo(cpuinfo *th);
void cpuinfo_getCPUInfo(cpuinfo *th);
uint64 cpuinfo_extractBits(uint64 value, uint32 begin, uint32 end) ;
uint32 cpuinfo_getNumCores(cpuinfo *th) ;
uint32 cpuinfo_getNumSockets(cpuinfo * th);
uint32 cpuinfo_getThreadsPerCore(cpuinfo *th);
char * cpuinfo_getCPUVendor(cpuinfo *th);
uint32 cpuinfo_getCPUFamily(cpuinfo *th);
uint32 cpuinfo_getCPUModel(cpuinfo *th);
uint32 cpuinfo_getNumCoreGeneralCounters(cpuinfo *th);
uint32 cpuinfo_getNumCoreFixedCounters(cpuinfo *th);
uint32 cpuinfo_getCoreGeneralCounterWidth(cpuinfo *th);
uint32 cpuinfo_getCoreFixedCounterWidth(cpuinfo *th);
uint32 cpuinfo_getPerformanceMonitorVersion(cpuinfo *th);


#endif
