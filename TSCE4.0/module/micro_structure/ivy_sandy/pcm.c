/*Inspur (Beijing) Electronic Information Industry Co., Ltd.
  
  Written by LiuYu
  
  File: pcm.cpp
  Version: V3.0.0
  Update: 2014-12-30
  
  File 'pcm.cpp' contains functions and methods used for setting 
  the event select registers and reading the result values from 
  the performance counters.
*/

//#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "types.h"
#include "pcm.h"
#include "cpuinfo.h"

//static member initialization
//PCM* PCM::instance = NULL;
PCM* instance = NULL;

void pcm_init(PCM *th)
{
    th->PCM = PCM_PCM;
    th->extractCoreGeneralCounterValue = PCM_extractCoreGeneralCounterValue;
    th->extractCoreFixedCounterValue = PCM_extractCoreFixedCounterValue;
    th->extractUncoreGeneralCounterValue = PCM_extractUncoreGeneralCounterValue;
    th->extractUncoreCBoCounterValue = PCM_extractUncoreCBoCounterValue;
    th->extractBits = PCM_extractBits;
    th->setBit = PCM_setBit;
    th->setEventRegister = PCM_setEventRegister;
    th->setUncoreEventRegister = PCM_setUncoreEventRegister;
    th->setUncoreCBoEventRegister = PCM_setUncoreCBoEventRegister;
    th->resetPMU = PCM_resetPMU;
   // th->getInstance = PCM_getInstance;
    th->getCounterValue = PCM_getCounterValue;
    th->getFixedCounterValue = PCM_getFixedCounterValue;
    th->getUncoreCounterValue = PCM_getUncoreCounterValue;
    th->getUncoreCBoCounterValue = PCM_getUncoreCBoCounterValue;
    th->getNumCores = PCM_getNumCores;
    th->getNumCoreGeneralCounters = PCM_getNumCoreGeneralCounters;
    th->getNumCoreFixedCounters = PCM_getNumCoreFixedCounters;
    th->getNumSockets = PCM_getNumSockets;
    th->getThreadsPerCore = PCM_getThreadsPerCore;
    th->getCPUModel = PCM_getCPUModel;
    th->UPCM = PCM_UPCM;
        
}

void PCM_PCM(struct class_PCM *th)
{ 
  th->NumCores = 0; 
  th->NumCoreGeneralCounters = 0;  
  th->NumCoreFixedCounters = 0; 
  th->NumSockets = 0; 
  th->CoreGeneralCounterWidth = 0;  
  th->CoreFixedCounterWidth = 0; 
  th->UncoreMCCounterWidth = 0; 
  th->UncoreCBoCounterWidth = 0; 
  th->PerformanceMonitorVersion = 0; 
  th->ThreadsPerCore = 0; 
  th->CPUFamily = 0; 
  th->CPUModel = 0; 
  th->MsrDevice = NULL;
  th->UncorePciMCPMON = NULL;
  th->UncoreMsrCBoPMON = NULL; 
  th->CoreCounterValue = NULL;
  th->CoreFixedCounterValue = NULL;
  th->UncorePciCounterValue = NULL;
  th->UncoreMsrCounterValue = NULL;

  /*//For 2-way SandyBridge-EP systems with HT off
  NumCores = 16;
  NumCoreGeneralCounters = 4;
  NumCoreFixedCounters = 3;
  NumSockets = 2;
  CoreGeneralCounterWidth = 48;
  CoreFixedCounterWidth = 48;
  UncoreMCCounterWidth = 48;
  PerformanceMonitorVersion = 3;
  ThreadsPerCore = 1;*/

  //get the cpu topological structure.
  cpuinfo CpuTopology;
  CpuTopology.cpuinfo = cpuinfo_cpuinfo;
  CpuTopology.cpuinfo(&CpuTopology);
  CpuTopology.getCPUInfo(&CpuTopology);
  th->NumCores = CpuTopology.getNumCores(&CpuTopology);
  th->NumCoreGeneralCounters = CpuTopology.getNumCoreGeneralCounters(&CpuTopology);
  th->NumCoreFixedCounters = CpuTopology.getNumCoreFixedCounters(&CpuTopology);
  th->NumSockets = CpuTopology.getNumSockets(&CpuTopology);
  th->CoreGeneralCounterWidth = CpuTopology.getCoreGeneralCounterWidth(&CpuTopology);
  th->CoreFixedCounterWidth = CpuTopology.getCoreFixedCounterWidth(&CpuTopology);
  th->PerformanceMonitorVersion = CpuTopology.getPerformanceMonitorVersion(&CpuTopology);
  th->ThreadsPerCore = CpuTopology.getThreadsPerCore(&CpuTopology);
  th->CPUFamily = CpuTopology.getCPUFamily(&CpuTopology);
  th->CPUModel = CpuTopology.getCPUModel(&CpuTopology);
  th->UncoreMCCounterWidth = 48;
  th->UncoreCBoCounterWidth = 44;

  printf_debug("%d:%d\t%d:%d:%d\t%d:%d-%d:%d-%d\n", \
             th->CPUFamily, th->CPUModel, \
             th->NumSockets, th->NumCores, th->ThreadsPerCore, \
             th->PerformanceMonitorVersion, th->NumCoreGeneralCounters, th->CoreGeneralCounterWidth, \
             th->NumCoreFixedCounters, th->CoreFixedCounterWidth);

  //check whether we support current cpu model.
  switch(th->CPUModel) {
  case SANDYBRIDGE_E5 :
    break;
  case IVYBRIDGE_E5 :
    break;
  case HASWELL_E5 :
    break;
  case BROADWELL_E5 :
    break;
  case KNL :
    break;
  default :
    printf("ERROR:\tUnsupported CPU!\n");
    exit(1);
  }

  th->MsrDevice = (msr **)malloc(sizeof(msr*) * th->NumCores);
  int i = 0;
  for (i = 0; i < th->NumCores; i++) {
    th->MsrDevice[i] = (msr *)malloc(sizeof( msr));
    msr_init(th->MsrDevice[i]);
    th->MsrDevice[i]->msr( th->MsrDevice[i], i);
 }

  th->CoreCounterValue = (uint64 *)malloc(sizeof(uint64) * th->NumCoreGeneralCounters);
  for (i = 0; i < th->NumCoreGeneralCounters; i++)
    th->CoreCounterValue[i] = 0;

  th->CoreFixedCounterValue = (uint64 *)malloc(sizeof(uint64) * th->NumCoreFixedCounters);

  //Only for SandyBridge-EP and IvyBridge-EP
  th->UncorePciMCPMON = (UncorePciMC**)malloc(sizeof(UncorePciMC*) * th->NumSockets);
  for (i = 0; i < th->NumSockets; i++) {
    //th->UncorePciMCPMON[i] = new UncorePciMC(i, th->NumSockets, th->CPUModel, th->NumCores);
        th->UncorePciMCPMON[i] = (UncorePciMC *)malloc(sizeof(UncorePciMC));
        UncorePciMC_init(th->UncorePciMCPMON[i]);
        th->UncorePciMCPMON[i]->UncorePciMC(th->UncorePciMCPMON[i], i, th->NumSockets, th->CPUModel, th->NumCores);
  }

  th->UncoreMsrCBoPMON = (UncoreMsrCBo **)malloc(sizeof(UncoreMsrCBo*) * th->NumSockets);
  for (i = 0; i < th->NumSockets; i++) {
    //th->UncoreMsrCBoPMON[i] = new UncoreMsrCBo(i, th->NumCores, th->NumSockets, th->ThreadsPerCore);
    th->UncoreMsrCBoPMON[i] = (UncoreMsrCBo *)malloc(sizeof(UncoreMsrCBo));
    UncoreMsrCBo_init(th->UncoreMsrCBoPMON[i]);
    th->UncoreMsrCBoPMON[i]->UncoreMsrCBo(th->UncoreMsrCBoPMON[i], i, th->NumCores, th->NumSockets, th->ThreadsPerCore);
  }

  //Array 'UncorePciCounterValue' only contains MC read and write values
  //th->UncorePciCounterValue = new uint64 [2];
  th->UncorePciCounterValue = (uint64 *)malloc(sizeof(uint64) * 2);

  //Array 'UncoreMsrCounterValue' only contains PCI-Express read and write 
  //bandwidth values
  //th->UncoreMsrCounterValue = new uint64 [2];
  th->UncoreMsrCounterValue = (uint64 *)malloc(sizeof(uint64) * 2);
}

void PCM_UPCM(PCM *th) {
  int i = 0;
  if (instance) {
    if (th->MsrDevice) {
      for (i = 0; i < th->NumCores; i++)
	if (th->MsrDevice[i])
	  free(th->MsrDevice[i]);
      free(th->MsrDevice);

      free(th->CoreCounterValue);

      free(th->CoreFixedCounterValue);
    }

    if (th->UncorePciMCPMON) {
      for (i = 0; i < th->NumSockets; i++)
	if (th->UncorePciMCPMON[i])
	  free(th->UncorePciMCPMON[i]);
      free(th->UncorePciMCPMON);

      free(th->UncorePciCounterValue);
    }

    if (th->UncoreMsrCBoPMON) {
      for (i = 0; i < th->NumSockets; i++)
	if (th->UncoreMsrCBoPMON[i])
	  free(th->UncoreMsrCBoPMON[i]);
      free(th->UncoreMsrCBoPMON);

      free(th->UncoreMsrCounterValue);
    }
  }
}

//static member function initialization
PCM* PCM_getInstance() {
  if (instance) return instance;

  //return instance = new PCM();
  instance = (PCM *)malloc(sizeof(PCM));
  pcm_init(instance);
  instance->PCM(instance);
  return instance;
}

void PCM_resetPMU(PCM *th) {
  int i = 0;
  int j = 0;
  for (i = 0; i < th->NumCores; i++) {
    //disable all counters by reset the global control register.
     th->MsrDevice[i]->write( th->MsrDevice[i], IA32_CR_PERF_GLOBAL_CTRL, 0);

    //Reset all counters to 0.
    for (j = 0; j <  th->NumCoreGeneralCounters; j++)
       th->MsrDevice[i]->write( th->MsrDevice[i], IA32_PERFEVTSEL0_ADDR + j, 0);
  }
}

void PCM_setEventRegister(PCM *th, EventAndUmask* parameters)  {
  //Check whether MSR is accessible
  if (!th->MsrDevice) {
    printf("Error while opening MSR device.\n");
    exit(1);
  }

  union GeneralEventSelectRegister GenEveSelReg;
  union FixedEventControlRegister FixEveCtlReg;
  int i = 0;
  int j = 0;

  for (i = 0; i < th->NumCores; i++) {
    //disable counters while programming
    th->MsrDevice[i]->write(th->MsrDevice[i], IA32_CR_PERF_GLOBAL_CTRL, 0);

    //set the core event select registers
    for (j = 0; j < th->NumCoreGeneralCounters; j++) {
      th->MsrDevice[i]->read(th->MsrDevice[i], IA32_PERFEVTSEL0_ADDR + j, &GenEveSelReg.value);

      //write the events and their umasks into select registers
      GenEveSelReg.fields.event_select = parameters[j].event;
      GenEveSelReg.fields.umask = parameters[j].umask;
      GenEveSelReg.fields.usr = 1;
      GenEveSelReg.fields.os = 1;
      GenEveSelReg.fields.edge = 0;
      GenEveSelReg.fields.pin_control = 0;
      GenEveSelReg.fields.apic_int = 0;
      GenEveSelReg.fields.any_thread = 0;
      GenEveSelReg.fields.enable = 1;
      GenEveSelReg.fields.invert = 0;
      GenEveSelReg.fields.cmask = 0;

      //clear the event counters
      th->MsrDevice[i]->write(th->MsrDevice[i], IA32_PMC0 + j, 0);
      //set the events into event select registers
      th->MsrDevice[i]->write(th->MsrDevice[i], IA32_PERFEVTSEL0_ADDR + j, GenEveSelReg.value);
    }

    //set the core fixed event control registers
    th->MsrDevice[i]->read(th->MsrDevice[i], IA32_CR_FIXED_CTR_CTRL, &FixEveCtlReg.value);

    //write the core fixed event control registers
    //CTR0
    FixEveCtlReg.fields.os0 = 1;
    FixEveCtlReg.fields.usr0 = 1;
    FixEveCtlReg.fields.any_thread0 = 0;
    FixEveCtlReg.fields.enable_pmi0 = 0;
    //CTR1
    FixEveCtlReg.fields.os1 = 1;
    FixEveCtlReg.fields.usr1 = 1;
    //sum the number of cycles from both logical cores on one physical core
    FixEveCtlReg.fields.any_thread1 = (th->PerformanceMonitorVersion >= 3 && th->ThreadsPerCore > 1) ? 1 : 0;
    FixEveCtlReg.fields.enable_pmi1 = 0;
    //CTR2
    FixEveCtlReg.fields.os2 = 1;
    FixEveCtlReg.fields.usr2 = 1;
    //sum the number of cycles from both logical cores on one physical core
    FixEveCtlReg.fields.any_thread2 = (th->PerformanceMonitorVersion >= 3 && th->ThreadsPerCore > 1) ? 1 : 0;
    FixEveCtlReg.fields.enable_pmi2 = 0;

    th->MsrDevice[i]->write(th->MsrDevice[i], IA32_CR_FIXED_CTR_CTRL, FixEveCtlReg.value);

    //set the global control register to start counting
    //please refer to:
    //"Intel 64 and IA-32 Architectures Software Developser's Manual"
    //Enable both the general counters and fixed counters
    uint64 GloEveCrl = (1ULL << 0) + (1ULL << 1) + (1ULL << 2) + (1ULL << 3) + (1ULL << 32) + (1ULL << 33) + (1ULL << 34);
    th->MsrDevice[i]->write(th->MsrDevice[i], IA32_CR_PERF_GLOBAL_CTRL, GloEveCrl);
  }
}

void PCM_setUncoreEventRegister(PCM *th, uint32 SocketId)  {

  //Set uncore PCI iMC registers for counting MC read and write BW
  th->UncorePciMCPMON[SocketId]->setCounterValue(th->UncorePciMCPMON[SocketId]);
}

void PCM_setUncoreCBoEventRegister(PCM *th, uint32 SocketId, uint64 Opcode, uint64 TidField)  {

  //Set uncore MSR CBo registers for counting PCI-Express read and write BW
  th->UncoreMsrCBoPMON[SocketId]->setCounterValue(th->UncoreMsrCBoPMON[SocketId], Opcode, TidField);
}

//This function read the core general counter values from CPU performance 
//counters. This function must be called first before calling 
//the core general counter functional object.
void PCM_getCounterValue(PCM *th, msr* CpuMsr) {
  int i = 0;
  for(i = 0; i < th->NumCoreGeneralCounters; i++) {
    CpuMsr->read(CpuMsr, IA32_PMC0 + i, &(th->CoreCounterValue[i]));

    th->CoreCounterValue[i] = th->extractCoreGeneralCounterValue(th, th->CoreCounterValue[i]);
  }
}

//This function read the core fixed counter values from CPU performance 
//counters. This function must be called first before calling 
//the core fixed counter functional object.
void PCM_getFixedCounterValue(PCM *th, uint32 CoreId) {
  PCM* PcmInst = PCM_getInstance();

  PcmInst->MsrDevice[CoreId]->read(PcmInst->MsrDevice[CoreId], INST_RETIRED_ANY_ADDR, &(th->CoreFixedCounterValue[0]));
  PcmInst->MsrDevice[CoreId]->read(PcmInst->MsrDevice[CoreId], CPU_CLK_UNHALTED_THREAD_ADDR, &(th->CoreFixedCounterValue[1]));
  PcmInst->MsrDevice[CoreId]->read(PcmInst->MsrDevice[CoreId], CPU_CLK_UNHALTED_REF_ADDR, &(th->CoreFixedCounterValue[2]));
  int i = 0;
  for(i = 0; i < th->NumCoreFixedCounters; i++)
    th->CoreFixedCounterValue[i] = th->extractCoreFixedCounterValue(th, th->CoreFixedCounterValue[i]);
}

//This function read the uncore Pci iMC PMON counter values. 
//This function must be called first before calling 
//the uncore Pci iMC PMON counter functional object.
void PCM_getUncoreCounterValue(PCM *th, uint32 SocketId) {
  //Data read from MC. Unit in cache line
  th->UncorePciCounterValue[0] = th->UncorePciMCPMON[SocketId]->getMCCounterRead(th->UncorePciMCPMON[SocketId]);
  th->UncorePciCounterValue[0] = th->extractUncoreGeneralCounterValue(th, th->UncorePciCounterValue[0]);
  //Data write to MC. Unit in cache line
  th->UncorePciCounterValue[1] = th->UncorePciMCPMON[SocketId]->getMCCounterWrite(th->UncorePciMCPMON[SocketId]);
  th->UncorePciCounterValue[1] = th->extractUncoreGeneralCounterValue(th, th->UncorePciCounterValue[1]);
}

//This function read the uncore Msr CBo PMON counter values.
//This function must be called first before calling 
//the uncore Msr CBo PMON counter functional object.
void PCM_getUncoreCBoCounterValue(PCM *th, uint32 SocketId) {
  //Data read from CBo PMON counters.
  th->UncoreMsrCounterValue[0] = th->UncoreMsrCBoPMON[SocketId]->getCBoCounterValue(th->UncoreMsrCBoPMON[SocketId]);
  th->UncoreMsrCounterValue[0] = th->extractUncoreCBoCounterValue(th, th->UncoreMsrCounterValue[0]);
}

uint64 PCM_extractCoreGeneralCounterValue(PCM *th, uint64 bitvalue) {
  if (th->CoreGeneralCounterWidth)
    return th->extractBits(th, bitvalue, 0, th->CoreGeneralCounterWidth - 1);

  return bitvalue;
}

uint64 PCM_extractCoreFixedCounterValue(PCM *th, uint64 bitvalue) {
  if (th->CoreFixedCounterWidth)
    return th->extractBits(th, bitvalue, 0, th->CoreFixedCounterWidth - 1);

  return bitvalue;
}

uint64 PCM_extractUncoreGeneralCounterValue(PCM *th, uint64 bitvalue) {
  if (th->UncoreMCCounterWidth)
    return th->extractBits(th, bitvalue, 0, th->UncoreMCCounterWidth - 1);

  return bitvalue;
}

uint64 PCM_extractUncoreCBoCounterValue(PCM *th, uint64 bitvalue) {
  if (th->UncoreCBoCounterWidth)
    return th->extractBits(th, bitvalue, 0, th->UncoreCBoCounterWidth - 1);

  return bitvalue;
}

uint64 PCM_extractBits(PCM *th, uint64 bitvalue, uint32 begin, uint32 end)
{
  uint64 extval = 0;
  uint32 beg1, end1;
  
  // Let the user reverse the order of beg & end.
  if (begin <= end) {
    beg1 = begin;
    end1 = end;
  }
  else {
    beg1 = end;
    end1 = begin;
  }

  extval = bitvalue >> beg1;
  extval = extval & th->setBit(th, beg1, end1);

  return extval;
}

uint64 PCM_setBit(PCM *th, uint32 begin, uint32 end)
{
  uint64 settedbit = 0;

  if (end == 63) {
    settedbit = (uint64)(-1);
  }
  else {
    settedbit = (1LL << (end + 1)) - 1;
  }
  
  settedbit = settedbit >> begin;
  
  return settedbit;
}

uint32 PCM_getNumCores(PCM *th) {
  return th->NumCores;
}

uint32 PCM_getNumCoreGeneralCounters(PCM *th)  {
  return th->NumCoreGeneralCounters;
}

uint32 PCM_getNumCoreFixedCounters(PCM *th)  {
  return th->NumCoreFixedCounters;
}

uint32 PCM_getNumSockets(PCM *th) {
  return th->NumSockets;
}

uint32 PCM_getThreadsPerCore(PCM *th) {
  return th->ThreadsPerCore;
}

uint32 PCM_getCPUModel(PCM *th) {
  return th->CPUModel;
}

void UncorePciMC_init(UncorePciMC *th)
{
    th->getMCCounterRead = UncorePciMC_getMCCounterRead;
    th->getMCCounterWrite = UncorePciMC_getMCCounterWrite;
    th->UncorePciMC = UncorePciMC_UncorePciMC;
    th->setCounterValue = UncorePciMC_setCounterValue;
    th->UUncorePciMC = UncorePciMC_UUncorePciMC;
    }

void UncorePciMC_UncorePciMC(UncorePciMC *th, uint32 SocketId, uint32 MaxSockets, \
                                                                                            uint32 ProcessorModel, uint32 MaxCores) 
{
  th->NumImcChannels = 0;
  th->PciMCDevices = NULL;

  uint32 bus = 0;

  if(2 == MaxSockets) {
    bus = SocketId ? 0xff : 0x7f;
  }
  else if (4 == MaxSockets) {
    const uint32 fourSocketBus[4] = { 0x3f, 0x7f, 0xbf, 0xff};
    bus =  fourSocketBus[SocketId];
  }
  else {
    printf("Error: A system with %d sockets is detected. Only 2- and 4-socket systems are supported.\n", MaxSockets);
	return 0;
  }

  //Sandy Bridge EP CPU has only one iMC.
  //Ivy Bridge EP CPU with 12 cores has two iMC.
  //Ivy Bridge EP CPU with 6, 8 or 10 cores only has one iMC.
  //Haswell EP CPU with 4, 6 or 8 cores only has one iMC.
  //Haswell EP CPU with 10, 12, 14, 16 or 18 cores has two iMC.
  if(((IVYBRIDGE_E5 == ProcessorModel) && (24 == MaxCores)) || 
     ((HASWELL_E5 == ProcessorModel) && (16 < MaxCores)) ||
     ((BROADWELL_E5 == ProcessorModel) && (20 < MaxCores)) ||
     ((KNL == ProcessorModel) && (20 < MaxCores)))
    th->PciMCDevices = (PciMC **)malloc(sizeof(PciMC *) * 8);
  else
    th->PciMCDevices = (PciMC **)malloc(sizeof(PciMC *) * 4);

  //For Sandy Bridge and Ivy Bridge CPU
  if ((SANDYBRIDGE_E5 == ProcessorModel) || (IVYBRIDGE_E5 == ProcessorModel)) {
    if (PciMC_existsPci(bus, 16, 0)) {
      //PciMCDevices[NumImcChannels++] = new PciMC(bus, 16, 0);
      th->PciMCDevices[th->NumImcChannels++] = (PciMC *)malloc(sizeof(PciMC));
      PciMC_init(th->PciMCDevices[th->NumImcChannels-1]);
      th->PciMCDevices[th->NumImcChannels-1]->PciMC(th->PciMCDevices[th->NumImcChannels-1], bus, 16, 0);
    }
    if (PciMC_existsPci(bus, 16, 1)) {
      //PciMCDevices[NumImcChannels++] = new PciMC(bus, 16, 1);
      th->PciMCDevices[th->NumImcChannels++] = (PciMC *)malloc(sizeof(PciMC));
      PciMC_init(th->PciMCDevices[th->NumImcChannels-1]);
      th->PciMCDevices[th->NumImcChannels-1]->PciMC(th->PciMCDevices[th->NumImcChannels-1], bus, 16, 1);
    }
    if (PciMC_existsPci(bus, 16, 4)) {
      //PciMCDevices[NumImcChannels++] = new PciMC(bus, 16, 4);
      th->PciMCDevices[th->NumImcChannels++] = (PciMC *)malloc(sizeof(PciMC));
      PciMC_init(th->PciMCDevices[th->NumImcChannels-1]);
      th->PciMCDevices[th->NumImcChannels-1]->PciMC(th->PciMCDevices[th->NumImcChannels-1], bus, 16, 4);
    }
    if (PciMC_existsPci(bus, 16, 5)) {
      //PciMCDevices[NumImcChannels++] = new PciMC(bus, 16, 5);
      th->PciMCDevices[th->NumImcChannels++] = (PciMC *)malloc(sizeof(PciMC));
      PciMC_init(th->PciMCDevices[th->NumImcChannels-1]);
      th->PciMCDevices[th->NumImcChannels-1]->PciMC(th->PciMCDevices[th->NumImcChannels-1], bus, 16, 5);
    }
    //For Ivy Bridge EP CPU with 12 cores.
    if ((IVYBRIDGE_E5 == ProcessorModel) && (24 == MaxCores)) {
      if (PciMC_existsPci(bus, 30, 0)) {
	//PciMCDevices[NumImcChannels++] = new PciMC(bus, 30, 0);
	 th->PciMCDevices[th->NumImcChannels++] = (PciMC *)malloc(sizeof(PciMC));
          PciMC_init(th->PciMCDevices[th->NumImcChannels-1]);
          th->PciMCDevices[th->NumImcChannels-1]->PciMC(th->PciMCDevices[th->NumImcChannels-1], bus, 30, 0);
      }
      if (PciMC_existsPci(bus, 30, 1)) {
	//PciMCDevices[NumImcChannels++] = new PciMC(bus, 30, 1);
	th->PciMCDevices[th->NumImcChannels++] = (PciMC *)malloc(sizeof(PciMC));
         PciMC_init(th->PciMCDevices[th->NumImcChannels-1]);
         th->PciMCDevices[th->NumImcChannels-1]->PciMC(th->PciMCDevices[th->NumImcChannels-1], bus, 30, 1);
      }
      if (PciMC_existsPci(bus, 30, 4)) {
	//PciMCDevices[NumImcChannels++] = new PciMC(bus, 30, 4);
	th->PciMCDevices[th->NumImcChannels++] = (PciMC *)malloc(sizeof(PciMC));
         PciMC_init(th->PciMCDevices[th->NumImcChannels-1]);
         th->PciMCDevices[th->NumImcChannels-1]->PciMC(th->PciMCDevices[th->NumImcChannels-1], bus, 30, 4);
      }
      if (PciMC_existsPci(bus, 30, 5)) {
	//PciMCDevices[NumImcChannels++] = new PciMC(bus, 30, 5);
	th->PciMCDevices[th->NumImcChannels++] = (PciMC *)malloc(sizeof(PciMC));
         PciMC_init(th->PciMCDevices[th->NumImcChannels-1]);
         th->PciMCDevices[th->NumImcChannels-1]->PciMC(th->PciMCDevices[th->NumImcChannels-1], bus, 30, 5);
      }
    }
  }
  //For Haswell E5 CPU
  //"HSX Uncore Programming Guide_521687_rev0.7"
  else if ((HASWELL_E5 ==ProcessorModel) ||  (BROADWELL_E5 == ProcessorModel) || (KNL == ProcessorModel)) {
    //For 4, 6, and 8 cores CPU
    if (PciMC_existsPci(bus, 20, 0)) {
      //PciMCDevices[NumImcChannels++] = new PciMC(bus, 20, 0);
      th->PciMCDevices[th->NumImcChannels++] = (PciMC *)malloc(sizeof(PciMC));
      PciMC_init(th->PciMCDevices[th->NumImcChannels-1]);
      th->PciMCDevices[th->NumImcChannels-1]->PciMC(th->PciMCDevices[th->NumImcChannels-1], bus, 20, 0);
    }
    if (PciMC_existsPci(bus, 20, 1)) {
      //PciMCDevices[NumImcChannels++] = new PciMC(bus, 20, 1);
      th->PciMCDevices[th->NumImcChannels++] = (PciMC *)malloc(sizeof(PciMC));
      PciMC_init(th->PciMCDevices[th->NumImcChannels-1]);
      th->PciMCDevices[th->NumImcChannels-1]->PciMC(th->PciMCDevices[th->NumImcChannels-1], bus, 20, 1);
    }
    if (PciMC_existsPci(bus, 21, 0)) {
      //PciMCDevices[NumImcChannels++] = new PciMC(bus, 21, 0);
      th->PciMCDevices[th->NumImcChannels++] = (PciMC *)malloc(sizeof(PciMC));
      PciMC_init(th->PciMCDevices[th->NumImcChannels-1]);
      th->PciMCDevices[th->NumImcChannels-1]->PciMC(th->PciMCDevices[th->NumImcChannels-1], bus, 21, 0);
    }
    if (PciMC_existsPci(bus, 21, 1)) {
      //PciMCDevices[NumImcChannels++] = new PciMC(bus, 21, 1);
      th->PciMCDevices[th->NumImcChannels++] = (PciMC *)malloc(sizeof(PciMC));
      PciMC_init(th->PciMCDevices[th->NumImcChannels-1]);
      th->PciMCDevices[th->NumImcChannels-1]->PciMC(th->PciMCDevices[th->NumImcChannels-1], bus, 21, 1);
    }
    //For 10, 12, 16 and 18 cores CPU.
    if (16 < MaxCores) {
      if (PciMC_existsPci(bus, 23, 0)) {
	//PciMCDevices[NumImcChannels++] = new PciMC(bus, 23, 0);
        th->PciMCDevices[th->NumImcChannels++] = (PciMC *)malloc(sizeof(PciMC));
        PciMC_init(th->PciMCDevices[th->NumImcChannels-1]);
        th->PciMCDevices[th->NumImcChannels-1]->PciMC(th->PciMCDevices[th->NumImcChannels-1], bus, 23, 0);
      }
      if (PciMC_existsPci(bus, 23, 1)) {
	//PciMCDevices[NumImcChannels++] = new PciMC(bus, 23, 1);
        th->PciMCDevices[th->NumImcChannels++] = (PciMC *)malloc(sizeof(PciMC));
        PciMC_init(th->PciMCDevices[th->NumImcChannels-1]);
        th->PciMCDevices[th->NumImcChannels-1]->PciMC(th->PciMCDevices[th->NumImcChannels-1], bus, 23, 1);
      }
      if (PciMC_existsPci(bus, 24, 0)) {
	//PciMCDevices[NumImcChannels++] = new PciMC(bus, 24, 0);
        th->PciMCDevices[th->NumImcChannels++] = (PciMC *)malloc(sizeof(PciMC));
        PciMC_init(th->PciMCDevices[th->NumImcChannels-1]);
        th->PciMCDevices[th->NumImcChannels-1]->PciMC(th->PciMCDevices[th->NumImcChannels-1], bus, 24, 0);
      }
      if (PciMC_existsPci(bus, 24, 1)) {
	//PciMCDevices[NumImcChannels++] = new PciMC(bus, 24, 1);
        th->PciMCDevices[th->NumImcChannels++] = (PciMC *)malloc(sizeof(PciMC));
        PciMC_init(th->PciMCDevices[th->NumImcChannels-1]);
        th->PciMCDevices[th->NumImcChannels-1]->PciMC(th->PciMCDevices[th->NumImcChannels-1], bus, 24, 1);
      }
    }
  }

  if (0 == th->NumImcChannels) {
    //delete [] PciMCDevices;
    free(th->PciMCDevices);

    th->PciMCDevices = NULL;

    exit(1);
  }

  if (th->NumImcChannels < 3) {
    printf("Warning: Only %d memory channels detected, must be 3 or 4.\n", th->NumImcChannels);
  }
}

void UncorePciMC_UUncorePciMC(UncorePciMC *th) 
{
  int i = 0;
  if (th->PciMCDevices) {
    for (i = 0; i < th->NumImcChannels; i++)
      if (th->PciMCDevices[i])
	free(th->PciMCDevices[i]);
    free(th->PciMCDevices);

    th->PciMCDevices = NULL;
  }
}

//set the Pci PMON register counter events and umasks.
//"Intel Xeon Processor E5-2600 Product Family Uncore Performance 
//Monitoring Guide"
void UncorePciMC_setCounterValue(UncorePciMC *th) {
  //'frz_en' is Freeze Enable.
  //If set to 1 and a freeze signal is received, the counters will 
  //be stopped or frozen, else the freeze signal will be ignored.
  uint32 frz_en = 1 << 16;
  //'frz' is Feeze.
  //If set to 1 and the frz_en is 1, the counters in this box will 
  //be frozen.
  uint32 frz = 1 << 8;

  //MC_CHy_PCI_PMON_CTL
  union MCChannelControlRegister ReadEventRegister;
  union MCChannelControlRegister WriteEventRegister;

  //Read from IMC
  ReadEventRegister.value = 0;
  ReadEventRegister.fields.enable = 1;
  ReadEventRegister.fields.event_select = CAS_COUNT;
  ReadEventRegister.fields.umask = CAS_COUNT_RD;

  //Write to IMC
  WriteEventRegister.value = 0;
  WriteEventRegister.fields.enable = 1;
  WriteEventRegister.fields.event_select = CAS_COUNT;
  WriteEventRegister.fields.umask = CAS_COUNT_WR;


  int i = 0;
  for (i = 0; i < th->NumImcChannels; i++) {
    //Counter Freeze Enable
    th->PciMCDevices[i]->write32(th->PciMCDevices[i], MC_CH_PCI_PMON_BOX_CTL, frz_en);
    //Counter Freeze
    th->PciMCDevices[i]->write32(th->PciMCDevices[i], MC_CH_PCI_PMON_BOX_CTL, frz_en + frz);

    //Check whether the Pci PMON MC counters works
    uint32 value = 0;
    th->PciMCDevices[i]->read32(th->PciMCDevices[i], MC_CH_PCI_PMON_BOX_CTL, &value);
    if ((frz_en + frz) != value) {
      printf("Error: Memory control counter seems not work. Channel  \
		%d  value is: %#x\n", i, value);
      printf("Please see BIOS options to enable the PCI PMON PMUs\n");
    }

    //Enable and set counter 0 to monitor data read to IMC.
    //PciMCDevices[i]->write32(MC_CH_PCI_PMON_CTL0, (1 << 22));
    th->PciMCDevices[i]->write32(th->PciMCDevices[i], MC_CH_PCI_PMON_CTL0, ReadEventRegister.value);

    //Enable and set counter 1 to monitor data write to IMC.
    //PciMCDevices[i]->write32(MC_CH_PCI_PMON_CTL1, (1 << 22));
    th->PciMCDevices[i]->write32(th->PciMCDevices[i], MC_CH_PCI_PMON_CTL1, WriteEventRegister.value);

    //Reset counters values
    th->PciMCDevices[i]->write32(th->PciMCDevices[i], MC_CH_PCI_PMON_CTR0, 0);
    th->PciMCDevices[i]->write32(th->PciMCDevices[i], MC_CH_PCI_PMON_CTR1, 0);

    //Unfreeze counters
    th->PciMCDevices[i]->write32(th->PciMCDevices[i], MC_CH_PCI_PMON_BOX_CTL, frz_en);
  }
}

//Return the data reads from IMC. Units in cache lines.
uint64 UncorePciMC_getMCCounterRead(UncorePciMC *th) {
  uint64 DataRead = 0;
  uint64 value = 0;

  int i = 0;
  for (i = 0; i < th->NumImcChannels; i++) {
    th->PciMCDevices[i]->read64(th->PciMCDevices[i], MC_CH_PCI_PMON_CTR0, &value);

    DataRead += value;
  }

  return DataRead;
}

//Return the data writes to IMC. Units in cache lines.
uint64 UncorePciMC_getMCCounterWrite(UncorePciMC *th) {
  uint64 DataWrite = 0;
  uint64 value = 0;
  int i = 0;

  for (i = 0; i < th->NumImcChannels; i++) {
    th->PciMCDevices[i]->read64(th->PciMCDevices[i], MC_CH_PCI_PMON_CTR1, &value);

    DataWrite += value;
  }

  return DataWrite;
}

void UncoreMsrCBo_init(UncoreMsrCBo *th)
{
    th->UncoreMsrCBo = UncoreMsrCBo_UncoreMsrCBo;
    th->UUncoreMsrCBo = UncoreMsrCBo_UUncoreMsrCBo;
    th->CX_MSR_PMON_BOX_CTL = UncoreMsrCBo_CX_MSR_PMON_BOX_CTL;
    th->CX_MSR_PMON_BOX_FILTER = UncoreMsrCBo_CX_MSR_PMON_BOX_FILTER;
    th->CX_MSR_PMON_BOX_FILTER1 = UncoreMsrCBo_CX_MSR_PMON_BOX_FILTER1;
    th->CX_MSR_PMON_CTLY = UncoreMsrCBo_CX_MSR_PMON_CTLY;
    th->CX_MSR_PMON_CTRY = UncoreMsrCBo_CX_MSR_PMON_CTRY;
    th->getCBoCounterValue = UncoreMsrCBo_getCBoCounterValue;
    th->setCBoFilterRegister = UncoreMsrCBo_setCBoFilterRegister;
    th->setCounterValue = UncoreMsrCBo_setCounterValue;
} 

void UncoreMsrCBo_UncoreMsrCBo(UncoreMsrCBo *th, uint32 SocketId, uint32 NumCores, 
			   uint32 NumSockets, uint32 ThreadsPerCore) 
{
  th->NumCBoxes = 0;
  th->RelatedMsrOfSocket = -1;

  //For each physical core has only one CBox.
  th->NumCBoxes = NumCores / NumSockets / ThreadsPerCore;

  //Specify one msr of the socket SocketId.
  th->RelatedMsrOfSocket = th->NumCBoxes * SocketId;
}

void UncoreMsrCBo_UUncoreMsrCBo(UncoreMsrCBo *th) {}

/*
  For technical deteals please contact Intel's reference manual.
  "Intel Xeon Processor E5-2600 Product Family Uncore Performance Monitoring Guide"
  "Ivy Bridge Uncore Performance Monitoring Reference Manual"
  "HSX Uncore Programming Guide_521687_rev0.7"
*/
void UncoreMsrCBo_setCounterValue(UncoreMsrCBo *th, uint64 Opcode, uint64 TidField) {
  PCM* PcmInst = PCM_getInstance();
  uint32 i = 0;
  if (PcmInst->MsrDevice) {
    for (i = 0; i < th->NumCBoxes; i++) {
      //freeze enable whole counters
      PcmInst->MsrDevice[th->RelatedMsrOfSocket]->write(PcmInst->MsrDevice[th->RelatedMsrOfSocket], \
                                                                                                th->CX_MSR_PMON_BOX_CTL(th, i),  \
						                                           CBO_MSR_PMON_BOX_CTL_FRZ_EN);
      //freeze whole counters
      PcmInst->MsrDevice[th->RelatedMsrOfSocket]->write(PcmInst->MsrDevice[th->RelatedMsrOfSocket], \
                                                            th->CX_MSR_PMON_BOX_CTL(th, i),  \
						      CBO_MSR_PMON_BOX_CTL_FRZ_EN + CBO_MSR_PMON_BOX_CTL_FRZ);

      //Check whether the msr devices are readable and been wrote correct.
      uint64 TempValue = 0;
      PcmInst->MsrDevice[th->RelatedMsrOfSocket]->read(PcmInst->MsrDevice[th->RelatedMsrOfSocket], \
                                                                                              th->CX_MSR_PMON_BOX_CTL(th, i), &TempValue);
      if ((TempValue & UNCORE_PMON_BOX_CTL_VALID_BITS_MASK) 
	  != (CBO_MSR_PMON_BOX_CTL_FRZ_EN + CBO_MSR_PMON_BOX_CTL_FRZ)) {
	printf("ERROR:\tCBO PMON counter seems not work. C%d__MSR_PMON_BOX_CTL=%#llx\n", i, TempValue);
      }

      //Set opcode into the CBo Filter Register
      th->setCBoFilterRegister(th, i, PcmInst->MsrDevice[th->RelatedMsrOfSocket], 
			   Opcode, PcmInst->getCPUModel(PcmInst));

      //Set the tid field of the Cn_MSR_PMON_BOX_FILTER0 register for Haswell-EP processor
      if ((HASWELL_E5 == PcmInst->getCPUModel(PcmInst) && 0 != TidField) ||
	(BROADWELL_E5 == PcmInst->getCPUModel(PcmInst) && 0 != TidField) ||
	(KNL == PcmInst->getCPUModel(PcmInst) && 0 != TidField) ) {
	PcmInst->MsrDevice[th->RelatedMsrOfSocket]->write(PcmInst->MsrDevice[th->RelatedMsrOfSocket], \
                                                                                                    th->CX_MSR_PMON_BOX_FILTER(th, i), TidField);
      }

      //Set control register Cx_MSR_PMON_CTL0 for counter Cx_MSR_PMON_CTR0
      PcmInst->MsrDevice[th->RelatedMsrOfSocket]->write(PcmInst->MsrDevice[th->RelatedMsrOfSocket], \
                                                                                                th->CX_MSR_PMON_CTLY(th, i, 0), 
                                            						      CBO_MSR_PMON_CTL_EN);
      //Set Event "TOR_INSERTS.OPCODE" into Cx_MSR_PMON_CTL0 with its event code and umask code
      uint64 TOR_INSERTS_OPCODE = CBO_MSR_PMON_CTL_EN 
	+ CBO_MSR_PMON_CTL_EVENT(0x35) 
	+ CBO_MSR_PMON_CTL_UMASK(1)
	+ (TidField?CBO_MSR_PMON_CTL_TID_EN:0ULL);
      PcmInst->MsrDevice[th->RelatedMsrOfSocket]->write(PcmInst->MsrDevice[th->RelatedMsrOfSocket], 
                                                                                                th->CX_MSR_PMON_CTLY(th, i, 0), \
                                                                                                TOR_INSERTS_OPCODE);

      //Reset whole CBo counters
      uint64 CBO_MSR_PMON_RESET = CBO_MSR_PMON_BOX_CTL_FRZ_EN 
	+ CBO_MSR_PMON_BOX_CTL_FRZ 
	+ CBO_MSR_PMON_BOX_CTL_RST_COUNTERS;
      PcmInst->MsrDevice[th->RelatedMsrOfSocket]->write(PcmInst->MsrDevice[th->RelatedMsrOfSocket], \
                                                                                                th->CX_MSR_PMON_BOX_CTL(th, i),  \
                                                                                                CBO_MSR_PMON_RESET);

      //Unfreeze whole CBo counters
      PcmInst->MsrDevice[th->RelatedMsrOfSocket]->write(PcmInst->MsrDevice[th->RelatedMsrOfSocket], \
                                                                                                th->CX_MSR_PMON_BOX_CTL(th, i), \
                                                                                                CBO_MSR_PMON_BOX_CTL_FRZ_EN);
    }
  }
  else {
   printf("ERROR:\tCan not find MSR devices for CBo PMON!\n");

    exit(1);
  }
}

/*
  For technical deteals please contact Intel's reference manual.
  "Intel Xeon Processor E5-2600 Product Family Uncore Performance Monitoring Guide"
  "Ivy Bridge Uncore Performance Monitoring Reference Manual"
  "HSX Uncore Programming Guide_521687_rev0.7"
*/
void UncoreMsrCBo_setCBoFilterRegister(UncoreMsrCBo *th, uint32 ICBo, 
					msr* MsrDevices, 
					uint64 Opcode, 
					uint32 ProcessorModel) {
  if (SANDYBRIDGE_E5 == ProcessorModel) {
    MsrDevices->write(MsrDevices, th->CX_MSR_PMON_BOX_FILTER(th, ICBo), 
		      SANDYBRIDGE_E5_CBO_MSR_PMON_BOX_FILTER_OPCODE(Opcode));
  }
  else if (IVYBRIDGE_E5 == ProcessorModel) {
    MsrDevices->write(MsrDevices, th->CX_MSR_PMON_BOX_FILTER1(th, ICBo), 
		      IVYBRIDGE_E5_CBO_MSR_PMON_BOX_FILTER1_OPCODE(Opcode));
  }
  else if ((HASWELL_E5 == ProcessorModel) ||
	 (BROADWELL_E5 == ProcessorModel) ||
	 (KNL == ProcessorModel)) {
    MsrDevices->write(MsrDevices, th->CX_MSR_PMON_BOX_FILTER1(th, ICBo),
		     HASWELL_E5_CBO_MSR_PMON_BOX_FILTER1_OPCODE(Opcode));
  }
}

uint64 UncoreMsrCBo_getCBoCounterValue(UncoreMsrCBo *th) {
  PCM* PcmInst = PCM_getInstance();
  uint64 Value = 0;
  uint32 i = 0;
  for (i = 0; i < th->NumCBoxes; i++) {
    uint64 TempValue = 0;
    PcmInst->MsrDevice[th->RelatedMsrOfSocket]->read(PcmInst->MsrDevice[th->RelatedMsrOfSocket], \
                                                                                            th->CX_MSR_PMON_CTRY(th, i, 0), &TempValue);
    Value += TempValue;
  }

  return Value;
}

uint32 UncoreMsrCBo_CX_MSR_PMON_CTRY(UncoreMsrCBo *th, uint32 ICBo, uint32 CounterID) 
{
  PCM* PcmInst = PCM_getInstance();

  if ((SANDYBRIDGE_E5 == PcmInst->getCPUModel(PcmInst)) ||
      (IVYBRIDGE_E5 == PcmInst->getCPUModel(PcmInst))) {
    return C0_MSR_PMON_CTR0 + ((CBO_MSR_STEP) * ICBo) + CounterID;
  }
  else if ((HASWELL_E5 == PcmInst->getCPUModel(PcmInst)) ||
	(BROADWELL_E5 == PcmInst->getCPUModel(PcmInst)) ||
	(KNL == PcmInst->getCPUModel(PcmInst))) {
    return HSW_C0_MSR_PMON_CTR0 + ((HSW_CBO_MSR_STEP) * ICBo) + CounterID;
  }
  return 0;
}

uint32 UncoreMsrCBo_CX_MSR_PMON_BOX_FILTER(UncoreMsrCBo *th, uint32 ICBo) {
  PCM* PcmInst = PCM_getInstance();

  if ((SANDYBRIDGE_E5 == PcmInst->getCPUModel(PcmInst)) ||
      (IVYBRIDGE_E5 == PcmInst->getCPUModel(PcmInst))) {
    return C0_MSR_PMON_BOX_FILTER + ((CBO_MSR_STEP) * ICBo);
  }
  else if ((HASWELL_E5 == PcmInst->getCPUModel(PcmInst)) ||
	(BROADWELL_E5 == PcmInst->getCPUModel(PcmInst)) ||
	(KNL == PcmInst->getCPUModel(PcmInst))) {
    return HSW_C0_MSR_PMON_BOX_FILTER + ((HSW_CBO_MSR_STEP) * ICBo);
  }
  return 0;
}

uint32 UncoreMsrCBo_CX_MSR_PMON_BOX_FILTER1(UncoreMsrCBo *th, uint32 ICBo)  {
  PCM* PcmInst = PCM_getInstance();

  if ((IVYBRIDGE_E5 == PcmInst->getCPUModel(PcmInst))) {

    return C0_MSR_PMON_BOX_FILTER1 + ((CBO_MSR_STEP) * ICBo);
  }
  else if ((HASWELL_E5 == PcmInst->getCPUModel(PcmInst)) ||
	(BROADWELL_E5 == PcmInst->getCPUModel(PcmInst)) ||
	(KNL == PcmInst->getCPUModel(PcmInst))) {
    return HSW_C0_MSR_PMON_BOX_FILTER1 + ((HSW_CBO_MSR_STEP) * ICBo);
  }
  return 0;
}

uint32 UncoreMsrCBo_CX_MSR_PMON_CTLY(UncoreMsrCBo *th, uint32 ICBo, uint32 CounterID)  {
  PCM* PcmInst = PCM_getInstance();

  if ((SANDYBRIDGE_E5 == PcmInst->getCPUModel(PcmInst)) || 
      (IVYBRIDGE_E5 == PcmInst->getCPUModel(PcmInst))) {
    return C0_MSR_PMON_CTL0 + ((CBO_MSR_STEP) * ICBo) + CounterID;
  }
  else if ((HASWELL_E5 == PcmInst->getCPUModel(PcmInst)) ||
	(BROADWELL_E5 == PcmInst->getCPUModel(PcmInst)) ||
	(KNL == PcmInst->getCPUModel(PcmInst))) {
    return HSW_C0_MSR_PMON_CTL0 + ((HSW_CBO_MSR_STEP) * ICBo) + CounterID;
  }
  return 0;
}

uint32 UncoreMsrCBo_CX_MSR_PMON_BOX_CTL(UncoreMsrCBo *th, uint32 ICBo){
  PCM* PcmInst = PCM_getInstance();

  if ((SANDYBRIDGE_E5 == PcmInst->getCPUModel(PcmInst)) || 
      (IVYBRIDGE_E5 == PcmInst->getCPUModel(PcmInst))) {
    return C0_MSR_PMON_BOX_CTL + ((CBO_MSR_STEP) * ICBo);
  }
  else if ((HASWELL_E5 == PcmInst->getCPUModel(PcmInst)) ||
	(BROADWELL_E5 == PcmInst->getCPUModel(PcmInst)) ||
	(KNL == PcmInst->getCPUModel(PcmInst)) ) {
    return HSW_C0_MSR_PMON_BOX_CTL + ((HSW_CBO_MSR_STEP) * ICBo);
  }
  return 0;
}

void CoreEventPCM_init(CoreEventPCM *th)
{
    th->getAVXPackedDoublePrecisionGFlops = CoreEventPCM_getAVXPackedDoublePrecisionGFlops;
    th->getAVXPackedSinglePrecisionGFlops =CoreEventPCM_getAVXPackedSinglePrecisionGFlops ;
    th->getCoreEvent = CoreEventPCM_getCoreEvent;
    th->getSSEPackedDoublePrecisionGFlops = CoreEventPCM_getSSEPackedDoublePrecisionGFlops;
    th->getSSEPackedSinglePrecisionGFlops = CoreEventPCM_getSSEPackedSinglePrecisionGFlops;
    th->getSSEScalarDoublePrecisionGFlops = CoreEventPCM_getSSEScalarDoublePrecisionGFlops;
    th->getSSEScalarSinglePrecisionGFlops = CoreEventPCM_getSSEScalarSinglePrecisionGFlops;
    th->getX87DoublePrecisionGFlops =CoreEventPCM_getX87DoublePrecisionGFlops ;
    th->setCoreEventSelectRegister = CoreEventPCM_setCoreEventSelectRegister;
    th->UCoreEventPCM = CoreEventPCM_UCoreEventPCM;
    th->CoreEventPCM = CoreEventPCM_CoreEventPCM;
}
void CoreEventPCM_CoreEventPCM(struct class_CoreEventPCM *th)
{
    th->X87DoublePrecision = 0;
    th->SSEPackedDoublePrecision = 0;
    th->SSEScalarDoublePrecision = 0;
    th->AVXPackedDoublePrecision = 0;
    th->SSEPackedSinglePrecision = 0;
    th->SSEScalarSinglePrecision = 0; 
    th->AVXPackedSinglePrecision = 0;
}
void  CoreEventPCM_UCoreEventPCM() {}


//set the core general event select register
void CoreEventPCM_setCoreEventSelectRegister(CoreEventPCM *th) {
  PCM* PcmInst = PCM_getInstance();

  EventAndUmask* parameters = (EventAndUmask *)malloc(sizeof(EventAndUmask) * PcmInst->getNumCoreGeneralCounters(PcmInst));

  if (PcmInst->getNumCoreGeneralCounters(PcmInst) < 4) {
    printf("Error: Core general counter is less then 4\n" );
    printf("getNumCoreGeneralCounters = %d\n", PcmInst->getNumCoreGeneralCounters(PcmInst));
    exit(1);
  }

  //X87 double precision
  parameters[0].event = FP_COMP_OPS_EXE;
  parameters[0].umask = X87;
  //SSE packed double precision
  parameters[1].event = FP_COMP_OPS_EXE;
  parameters[1].umask = SSE_FP_PACKED_DOUBLE;
  //SSE scalar double precision
  parameters[2].event = FP_COMP_OPS_EXE;
  parameters[2].umask = SSE_SCALAR_DOUBLE;
  //AVX packed double precision (only for SandyBridge)
  parameters[3].event = SIMD_FP_256;
  parameters[3].umask = AVX_PACKED_DOUBLE;

  if (((SANDYBRIDGE_E5 == PcmInst->getCPUModel(PcmInst)) ||
       (IVYBRIDGE_E5 == PcmInst->getCPUModel(PcmInst)) ||
       (HASWELL_E5 == PcmInst->getCPUModel(PcmInst)) ||
	(BROADWELL_E5 == PcmInst->getCPUModel(PcmInst)) ||
	(KNL == PcmInst->getCPUModel(PcmInst)))
          && (1 == PcmInst->getThreadsPerCore(PcmInst))) {
    //SSE packed single precision
    parameters[4].event = FP_COMP_OPS_EXE;
    parameters[4].umask = SSE_FP_PACKED_SINGLE;
    //SSE scalar single precision
    parameters[5].event = FP_COMP_OPS_EXE;
    parameters[5].umask = SSE_SCALAR_SINGLE;
    //AVX packed single precision (only for SandyBridge)
    parameters[6].event = SIMD_FP_256;
    parameters[6].umask = AVX_PACKED_SINGLE;
  }

  //set the event select registers
  PcmInst->setEventRegister(PcmInst, parameters);
}

//Before calling the function, method 'getCounterValue' in 
// class PCM must be called first to get the event counter value
void CoreEventPCM_getCoreEvent(CoreEventPCM *th) {
  PCM* PcmInst = PCM_getInstance();

  if (PcmInst->getNumCoreGeneralCounters(PcmInst) < 4) {
    printf("Error: Core general counter is less then 4\n");
    exit(1);
  }

  th->X87DoublePrecision = PcmInst->CoreCounterValue[0];
  th->SSEPackedDoublePrecision = PcmInst->CoreCounterValue[1];
  th->SSEScalarDoublePrecision = PcmInst->CoreCounterValue[2];
  th->AVXPackedDoublePrecision = PcmInst->CoreCounterValue[3];
  if(((SANDYBRIDGE_E5 == PcmInst->getCPUModel(PcmInst)) ||
      (IVYBRIDGE_E5 == PcmInst->getCPUModel(PcmInst)) ||
      (HASWELL_E5 == PcmInst->getCPUModel(PcmInst)) ||
	(BROADWELL_E5 == PcmInst->getCPUModel(PcmInst)) ||
	(KNL == PcmInst->getCPUModel(PcmInst))) 
     && (1== PcmInst->getThreadsPerCore(PcmInst))) {
    th->SSEPackedSinglePrecision = PcmInst->CoreCounterValue[4];
    th->SSEScalarSinglePrecision = PcmInst->CoreCounterValue[5];
    th->AVXPackedSinglePrecision = PcmInst->CoreCounterValue[6];
  }
}

//unit is GFlops
float CoreEventPCM_getX87DoublePrecisionGFlops(CoreEventPCM *th, const CoreEventPCM old)  {
  return (th->X87DoublePrecision - old.X87DoublePrecision) / (double)1000000000;
}

//unit is GFlops
float CoreEventPCM_getSSEPackedDoublePrecisionGFlops(CoreEventPCM *th, const CoreEventPCM old)  {
  return (th->SSEPackedDoublePrecision - old.SSEPackedDoublePrecision) * 2.0 / (double)1000000000;
}

//unit is GFlops
float CoreEventPCM_getSSEScalarDoublePrecisionGFlops(CoreEventPCM *th, const CoreEventPCM old) {
  return (th->SSEScalarDoublePrecision - old.SSEScalarDoublePrecision) / (double)1000000000;
}

//unit is GFlops
float CoreEventPCM_getAVXPackedDoublePrecisionGFlops(CoreEventPCM *th, const CoreEventPCM old) {
  return (th->AVXPackedDoublePrecision - old.AVXPackedDoublePrecision) * 4.0 / (double)1000000000;
}

//unit is GFlops
float CoreEventPCM_getSSEPackedSinglePrecisionGFlops(CoreEventPCM *th, const CoreEventPCM old) {
  return (th->SSEPackedSinglePrecision - old.SSEPackedSinglePrecision) * 2.0 / (double)1000000000;
}

//unit is GFlops
float CoreEventPCM_getSSEScalarSinglePrecisionGFlops(CoreEventPCM *th, const CoreEventPCM old) {
  return (th->SSEScalarSinglePrecision - old.SSEScalarSinglePrecision) / (double)1000000000;
}

//unit is GFlops
float CoreEventPCM_getAVXPackedSinglePrecisionGFlops(CoreEventPCM *th, const CoreEventPCM old) {
  return (th->AVXPackedSinglePrecision - old.AVXPackedSinglePrecision) * 4.0 / (double)1000000000;
}

void CoreFixedEventPCM_init(CoreFixedEventPCM *th)
{
    th->CoreFixedEventPCM = CoreFixedEventPCM_CoreFixedEventPCM;
    th->getCoreFixedEvent = CoreFixedEventPCM_getCoreFixedEvent;
    th->getCPI = CoreFixedEventPCM_getCPI;
    th->getCpuClockUnhaltedRef = CoreFixedEventPCM_getCpuClockUnhaltedRef;
    th->getCpuClockUnhaltedThread = CoreFixedEventPCM_getCpuClockUnhaltedThread ;
    th->getInstructionRetiredAny = CoreFixedEventPCM_getInstructionRetiredAny;
    th->UCoreFixedEventPCM = CoreFixedEventPCM_UCoreFixedEventPCM;
}

void CoreFixedEventPCM_CoreFixedEventPCM(struct class_CoreFixedEventPCM *th)
{ 
    th->InstructionRetiredAny = 0;
    th->CpuClockUnhaltedThread = 0;
    th->CpuClockUnhaltedRef = 0;
}
void CoreFixedEventPCM_UCoreFixedEventPCM() {}


//Before calling the function, method 'getFixedCounterValue' in 
//class PCM must be called first to get the event counter value
void CoreFixedEventPCM_getCoreFixedEvent(CoreFixedEventPCM *th) {
  PCM* PcmInst = PCM_getInstance();

  if (PcmInst->getNumCoreFixedCounters(PcmInst) < 3) {
    printf("Error: Core general counter is less then 3\n");
    exit(1);
  }

  th->InstructionRetiredAny = PcmInst->CoreFixedCounterValue[0];
  th->CpuClockUnhaltedThread = PcmInst->CoreFixedCounterValue[1];
  th->CpuClockUnhaltedRef = PcmInst->CoreFixedCounterValue[2];
}

uint64 CoreFixedEventPCM_getInstructionRetiredAny(CoreFixedEventPCM *th, const CoreFixedEventPCM old) {
  return th->InstructionRetiredAny - old.InstructionRetiredAny;
}

uint64 CoreFixedEventPCM_getCpuClockUnhaltedThread(CoreFixedEventPCM *th, const CoreFixedEventPCM old) {
  return th->CpuClockUnhaltedThread - old.CpuClockUnhaltedThread;
}

uint64 CoreFixedEventPCM_getCpuClockUnhaltedRef(CoreFixedEventPCM *th, const CoreFixedEventPCM old) {
  return th->CpuClockUnhaltedRef - old.CpuClockUnhaltedRef;
}

float CoreFixedEventPCM_getCPI(CoreFixedEventPCM *th, const CoreFixedEventPCM old)  {
  if (0 != th->getInstructionRetiredAny(th, old))
    return th->getCpuClockUnhaltedThread(th, old) * 1.0 / th->getInstructionRetiredAny(th, old);
  else {
    printf("Error in CPU instruction counting.\n");
    exit(1);
  }
}

void UncoreEventPCM_init(UncoreEventPCM *th)
{
    th->UncoreEventPCM = UncoreEventPCM_UncoreEventPCM;
    th->UUncoreEventPCM = UncoreEventPCM_UUncoreEventPCM;
    th->getBandWidthRead = UncoreEventPCM_getBandWidthRead; 
    th->getBandWidthWrite = UncoreEventPCM_getBandWidthWrite;
    th->getPCIeBandWidthRead = UncoreEventPCM_getPCIeBandWidthRead;
    th->getPCIeBandWidthWrite = UncoreEventPCM_getPCIeBandWidthWrite;
    th->getPCIeBWEvent = UncoreEventPCM_getPCIeBWEvent;
    th->getUncoreCBoEvent = UncoreEventPCM_getUncoreCBoEvent;
    th->getUncoreEvent = UncoreEventPCM_getUncoreEvent;
}
void UncoreEventPCM_UncoreEventPCM(struct class_UncoreEventPCM *th)
{
    th->BandWidthRead = 0;
    th->BandWidthWrite = 0;
    th->PCIeBandWidthRead = 0;
    th->PCIeBandWidthWrite = 0;
    th->PCIeBandWidth = (uint64 *)malloc(sizeof(uint64) * 2);
}
void UncoreEventPCM_UUncoreEventPCM(struct class_UncoreEventPCM *th) 
{
        if (NULL != th->PCIeBandWidth) {
            free(th->PCIeBandWidth);
        }
}


//Before calling the function, method 'getUncoreCounterValue' 
//in class PCM must be called first to get the Pci iMC PMON 
//event counter value
void UncoreEventPCM_getUncoreEvent(UncoreEventPCM *th) {
  PCM* PcmInst = PCM_getInstance();

  //Unit is byte
  th->BandWidthRead = PcmInst->UncorePciCounterValue[0] * 64;
  th->BandWidthWrite = PcmInst->UncorePciCounterValue[1] * 64;
}

//This function is used to sample the PCI-Express BW and should 
//be called in another process/thread
//For more information please refer to 
//"HSX Uncore Programming Guide_521687_rev0.7"
//void UncoreEventPCM_getPCIeBWEvent(UncoreEventPCM *th) {
void *UncoreEventPCM_getPCIeBWEvent(void *th) {

  //For SandyBridge_EP and IvyBridge_EP processor
  struct PCIeBWEvent {
    // PCI-Express read events from memory
    uint64 PCIePRd;
    uint64 PCIeRdCur;
    uint64 PCIeNSRd;
    //PCI-Express write events to memory
    uint64 PCIeWiLF;
    uint64 PCIeItoM;
    uint64 PCIeNSWr;
    uint64 PCIeNSWrF;
  };
  //For Haswell_EP processor
  struct PCIeBWEventHSW {
    // PCI-Express read events from memory
    uint64 PCIeRdCur;
    uint64 PRdwtid;
    uint64 PRd;
    // PCI-Express write events to memory
    uint64 ItoM;
    uint64 PCIeNSWr;
  };

  //For SNB-EP and IVY-EP processor
  struct PCIeBWEvent PCIeBWValue;
  //For Haswell-EP processor
  struct PCIeBWEventHSW PCIeBWValueHSW;

  PCM* PcmInst = PCM_getInstance();

  uint32 NumValues;
  if (SANDYBRIDGE_E5 == PcmInst->getCPUModel(PcmInst) ||
      IVYBRIDGE_E5 == PcmInst->getCPUModel(PcmInst))
    NumValues = sizeof(struct PCIeBWEvent) / sizeof(uint64);
  else if ((HASWELL_E5 == PcmInst->getCPUModel(PcmInst)) ||
	BROADWELL_E5 == PcmInst->getCPUModel(PcmInst) ||
	KNL == PcmInst->getCPUModel(PcmInst))
    NumValues = sizeof(struct PCIeBWEventHSW) / sizeof(uint64);
  
  float TimeStep = 1.0 / NumValues * 1000 * 1000;
  uint64 TimeInterval = 0;
  
  if ((uint32)TimeStep * NumValues < 1000 * 1000) 
    TimeInterval = (uint32)TimeStep + 1; 
  else 
    TimeInterval = (uint32)TimeStep;

  while(1) {
    //For SNB-EP and IVY-EP processor
    if (SANDYBRIDGE_E5 == PcmInst->getCPUModel(PcmInst) ||
	IVYBRIDGE_E5 == PcmInst->getCPUModel(PcmInst)) {
      memset(&PCIeBWValue, 0, sizeof(PCIeBWValue));

      PCIeBWValue.PCIePRd = ((UncoreEventPCM *)th)->getUncoreCBoEvent((UncoreEventPCM *)th, PCIPRD, TimeInterval, 0);
      PCIeBWValue.PCIeRdCur = ((UncoreEventPCM *)th)->getUncoreCBoEvent((UncoreEventPCM *)th, PCIRDCUR, TimeInterval, 0);
      PCIeBWValue.PCIeNSRd = ((UncoreEventPCM *)th)->getUncoreCBoEvent((UncoreEventPCM *)th, PCINSRD, TimeInterval, 0);
      PCIeBWValue.PCIeWiLF = ((UncoreEventPCM *)th)->getUncoreCBoEvent((UncoreEventPCM *)th, PCIWILF, TimeInterval, 0);
      PCIeBWValue.PCIeItoM = ((UncoreEventPCM *)th)->getUncoreCBoEvent((UncoreEventPCM *)th, PCIITOM, TimeInterval, 0);
      PCIeBWValue.PCIeNSWr = ((UncoreEventPCM *)th)->getUncoreCBoEvent((UncoreEventPCM *)th, PCINSWR, TimeInterval, 0);
      PCIeBWValue.PCIeNSWrF = ((UncoreEventPCM *)th)->getUncoreCBoEvent((UncoreEventPCM *)th, PCINSWRF, TimeInterval, 0);

      sem_wait(&IvySandy_pcie_sem_put); 
      ((UncoreEventPCM *)th)->PCIeBandWidth[0] = (PCIeBWValue.PCIePRd + PCIeBWValue.PCIeRdCur 
			  + PCIeBWValue.PCIeNSRd) * 64 * NumValues;
      ((UncoreEventPCM *)th)->PCIeBandWidth[1] = (PCIeBWValue.PCIeWiLF + PCIeBWValue.PCIeItoM 
			  + PCIeBWValue.PCIeNSWr + PCIeBWValue.PCIeNSWrF) * 64 * NumValues;
    sem_post(&IvySandy_pcie_sem_get);
    }
    //For Haswell-EP processor
    else if ((HASWELL_E5 == PcmInst->getCPUModel(PcmInst)) ||
		 BROADWELL_E5 == PcmInst->getCPUModel(PcmInst) ||
		 KNL == PcmInst->getCPUModel(PcmInst)) {
      memset(&PCIeBWValueHSW, 0, sizeof(PCIeBWValueHSW));

      //For more information please refer to 
      //"HSX Uncore Programming Guide_521687_rev0.7"
      //Page 51-52
      PCIeBWValueHSW.PCIeRdCur = ((UncoreEventPCM *)th)->getUncoreCBoEvent((UncoreEventPCM *)th, PCIRDCUR, TimeInterval, 0);
      PCIeBWValueHSW.PRdwtid = ((UncoreEventPCM *)th)->getUncoreCBoEvent((UncoreEventPCM *)th, PRD, TimeInterval, PRDTID);
      PCIeBWValueHSW.PRd = ((UncoreEventPCM *)th)->getUncoreCBoEvent((UncoreEventPCM *)th, PRD, TimeInterval, 0);
      PCIeBWValueHSW.ItoM = ((UncoreEventPCM *)th)->getUncoreCBoEvent((UncoreEventPCM *)th, ITOM, TimeInterval, ITOMTID);
      PCIeBWValueHSW.PCIeNSWr = ((UncoreEventPCM *)th)->getUncoreCBoEvent((UncoreEventPCM *)th, PCINSWR, TimeInterval, 0);

      sem_wait(&IvySandy_pcie_sem_put); 
      ((UncoreEventPCM *)th)->PCIeBandWidth[0] = (PCIeBWValueHSW.PCIeRdCur + PCIeBWValueHSW.PRdwtid 
			  + PCIeBWValueHSW.PRd) * 64 * NumValues;
      ((UncoreEventPCM *)th)->PCIeBandWidth[1] = (PCIeBWValueHSW.ItoM + PCIeBWValueHSW.PCIeNSWr) * 64 * NumValues;
      sem_post(&IvySandy_pcie_sem_get); 
    }
    
  }
}

//This function is used to get the uncore Msr CBo PMON counter's 
//value by given 'Opcode' and 'TidField'
uint64 UncoreEventPCM_getUncoreCBoEvent(UncoreEventPCM *th, uint64 Opcode,  
					 uint32 TimeInterval, 
					 const uint64 TidField) {
  PCM* PcmInst = PCM_getInstance();

  uint64 CounterValue = 0;
  int i = 0;
  for (i = 0; i < PcmInst->getNumSockets(PcmInst); i++) {
    //set uncore Msr CBo PMON register
    PcmInst->setUncoreCBoEventRegister(PcmInst, i, Opcode, TidField);
    //read uncore Msr CBo PMON event counters
    PcmInst->getUncoreCBoCounterValue(PcmInst, i);
  }

  usleep(TimeInterval);

  for (i = 0; i < PcmInst->getNumSockets(PcmInst); i++) {
    //read uncore Msr CBo PMON event counters
    PcmInst->getUncoreCBoCounterValue(PcmInst, i);

    CounterValue += PcmInst->UncoreMsrCounterValue[0];
  }

  return CounterValue;
}

//The bandwidth read from MC is define in unit GB/s.
float UncoreEventPCM_getBandWidthRead(UncoreEventPCM *th, const UncoreEventPCM old) {
  return (th->BandWidthRead - old.BandWidthRead) * 1.0 / 1024 / 1024 / 1024;
}

//The bandwidth write to MC is define in unit GB/s.
float UncoreEventPCM_getBandWidthWrite(UncoreEventPCM *th, const UncoreEventPCM old)  {
  return (th->BandWidthWrite - old.BandWidthWrite) * 1.0 / 1024 / 1024 / 1024;
}

//The PCI-Express bandwidth reading from memory in unit GB/s.
float UncoreEventPCM_getPCIeBandWidthRead(UncoreEventPCM *th) {
  //return th->PCIeBandWidth[0] * 1.0 / 1024 / 1024 / 1024;
  return th->PCIeBandWidth[0] * 1.0 / 1024 / 1024;
}

//The PCI-Express bandwidth writing to memory in unit GB/s.
float UncoreEventPCM_getPCIeBandWidthWrite(UncoreEventPCM *th) {
  //return th->PCIeBandWidth[1] * 1.0 / 1024 / 1024 / 1024; 
  return th->PCIeBandWidth[1] * 1.0 / 1024 / 1024; 
}
