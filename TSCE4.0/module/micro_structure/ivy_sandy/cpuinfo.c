/*Inspur (Beijing) Electronic Information Industry Co., Ltd.
  
  Written by LiuYu
  
  File: cpuinfo.cpp
  Version: V1.0 alpha
  Update: 2013-02-06
  
  Class cpuinfo is used for getting CPU 
  informaction from the host, such as, number of cores, 
  number of sockets, number of core general counters and so on.
*/

//#include <iostream>
//#include <fstream>
//#include <string>
//#include <bitset>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "cpuinfo.h"

void cpuinfo_cpuinfo(cpuinfo *th)
{
  th->NumCores = 0; 
  th->NumSockets = 0; 
  th->ThreadsPerCore = 0; 
  strncpy(th->CPUVendor, "", sizeof(th->CPUVendor)); 
  th->CPUFamily= -1; 
  th->CPUModel = -1;
  th->NumCoreGeneralCounters = -1; 
  th->NumCoreFixedCounters = -1; 
  th->CoreGeneralCounterWidth = -1; 
  th->CoreFixedCounterWidth = -1;
  th->PerformanceMonitorVersion = -1;
  //th->cpuinfo = cpuinfo_cpuinfo;
  th->Ucpuinfo = cpuinfo_Ucpuinfo;
  th->getCPUInfo = cpuinfo_getCPUInfo;
  th->extractBits = cpuinfo_extractBits ;
  th->getNumCores = cpuinfo_getNumCores ;
  th->getNumSockets = cpuinfo_getNumSockets;
  th->getThreadsPerCore  = cpuinfo_getThreadsPerCore;
  th->getCPUVendor  = cpuinfo_getCPUVendor;
  th->getCPUFamily  = cpuinfo_getCPUFamily;
  th->getCPUModel  = cpuinfo_getCPUModel;
  th->getNumCoreGeneralCounters  = cpuinfo_getNumCoreGeneralCounters;
  th->getNumCoreFixedCounters  = cpuinfo_getNumCoreFixedCounters;
  th->getCoreGeneralCounterWidth  = cpuinfo_getCoreGeneralCounterWidth;
  th->getCoreFixedCounterWidth  = cpuinfo_getCoreFixedCounterWidth;
  th->getPerformanceMonitorVersion  = cpuinfo_getPerformanceMonitorVersion;
}

void cpuinfo_Ucpuinfo(cpuinfo *th) {}

void cpuinfo_getCPUInfo(cpuinfo *th) {
  FILE *fp = fopen("/proc/cpuinfo", "r");
  if (!fp){
    printf("ERROR:\tCan not open file /proc/cpuinfo.\n");
    exit(1);
  }

  char buf[1024];
  uint32 tempsocketid = 0;
  uint32 cpuid = 0;
  char Vendor[100];

  while (0 != fgets(buf, 1024, fp)) {
    if (0 == strncmp(buf, "processor", sizeof("processor") - 1)) {
      th->NumCores++;
    }
    if (0 == strncmp(buf, "physical id", sizeof("physical id") - 1)) {
      sscanf(buf, "physical id\t: %d", &tempsocketid);
      if (th->NumSockets < tempsocketid)
	th->NumSockets++;
    }
    if (0 == strncmp(buf, "core id", sizeof("core id") - 1)) {
      sscanf(buf, "core id\t: %d", &cpuid);
      if (0 == tempsocketid && 0 == cpuid)
	th->ThreadsPerCore++;
    }
    if (0 == (th->NumCores - 1)) {
      if (0 == strncmp(buf, "model", sizeof("model") - 1)) {
	sscanf(buf, "model\t: %d", &th->CPUModel);
      }
      if (0 == strncmp(buf, "cpu family", sizeof("cpu family") - 1)) {
      	sscanf(buf, "cpu family\t: %d", &th->CPUFamily);
      }
      if (0 == strncmp(buf, "vendor_id", sizeof("vendor_id") - 1)) {
	sscanf(buf, "vendor_id\t: %s", Vendor);
	strncpy(th->CPUVendor, Vendor, sizeof(th->CPUVendor));

	if (0 != strncmp(th->CPUVendor, "GenuineIntel", 12)) {
	  printf("ERROR:\tNot supported platform!\n");
	  exit(1);
	}
      }
    }
  }

  fclose(fp);

  th->NumSockets++;

  uint32 eax, ebx, ecx, edx;

  GETCPUID(0, eax, ebx, ecx, edx);
  uint32 MaxId = eax;

  if (MaxId >= 0xa) {
    GETCPUID(0xa, eax, ebx, ecx, edx);

    th->PerformanceMonitorVersion = th->extractBits(eax, 0, 7);
    th->NumCoreGeneralCounters = th->extractBits(eax, 8, 15);
    th->CoreGeneralCounterWidth = th->extractBits(eax, 16, 23);

    if (th->PerformanceMonitorVersion > 1) {
      th->NumCoreFixedCounters = th->extractBits(edx, 0, 4);
      th->CoreFixedCounterWidth = th->extractBits(edx, 5, 12);
    }
  }

}

uint64 cpuinfo_extractBits(uint64 value, uint32 begin, uint32 end) {
  if (begin > end) {
    uint32 temp = end;
    end = begin;
    begin = temp;
  }
  uint64 mask = (uint64)pow(2, end-begin+1) - 1;
  return (value >> begin) & mask;
#if 0
  std::bitset<64> bita(value);
  std::bitset<64> bitb(0);

  for (uint32 i = begin; i <= end; i++)
    bitb[i - begin] = bita[i];

  return bitb.to_ulong();
#endif  
}

uint32 cpuinfo_getNumCores(cpuinfo *th) {
  return th->NumCores;
}

uint32 cpuinfo_getNumSockets(cpuinfo *th)  {
  return th->NumSockets;
}

uint32 cpuinfo_getThreadsPerCore(cpuinfo *th)  {
  return th->ThreadsPerCore;
}

char * cpuinfo_getCPUVendor(cpuinfo *th)  {
  return th->CPUVendor;
}

uint32 cpuinfo_getCPUFamily(cpuinfo *th)  {
  return th->CPUFamily;
}

uint32 cpuinfo_getCPUModel(cpuinfo *th)  {
  return th->CPUModel;
}

uint32 cpuinfo_getNumCoreGeneralCounters(cpuinfo *th)  {
  return th->NumCoreGeneralCounters;
}

uint32 cpuinfo_getNumCoreFixedCounters(cpuinfo *th)  {
  return th->NumCoreFixedCounters;
}

uint32 cpuinfo_getCoreGeneralCounterWidth(cpuinfo *th)  {
  return th->CoreGeneralCounterWidth;
}

uint32 cpuinfo_getCoreFixedCounterWidth(cpuinfo *th)  {
  return th->CoreFixedCounterWidth;
}

uint32 cpuinfo_getPerformanceMonitorVersion(cpuinfo *th) {
  return th->PerformanceMonitorVersion;
}
