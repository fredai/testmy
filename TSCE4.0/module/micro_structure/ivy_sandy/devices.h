/*Inspur (Beijing) Electronic Information Industry Co., Ltd.
  
  Written by LiuYu
  
  File: devices.h
  Version: V1.0 alpha
  Update: 2013-01-26
  
  Head file of devices.cpp which is used for reading and writing msr 
  ('/dev/cpu/?/msr'), pci('/proc/bus/pci/??/??.?') and mem('/dev/mem') 
  devices and so on. 
*/

#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "types.h"

//Operating cpu msr device: /dev/cpu/?/msr.

typedef struct class_msr msr;

struct class_msr {
  int fd;
  int cpu_id;

  void (*msr)(struct class_msr *th, int cpu);
  void (*Umsr)(struct class_msr *th);

  int (*read)(struct class_msr *th, uint64 event, uint64* value);
  int (*write)(struct class_msr *th, uint64 event, uint64 value);
};

//Operating pci and mem device: /proc/bus/pci/??/??.? and /dev/mem.

typedef struct class_PciMC PciMC;

struct class_PciMC {
  int32 fd;
  uint32 bus;
  uint32 device;
  uint32 function;
  uint64 BaseAddr;

  //forbidden
  /* jff note start
   * PciMC(struct class_PciMC *th);
   * jff note end 
   */

  void (*PciMC)(struct class_PciMC *th, uint32 bus_, uint32 device_, uint32 function_);

  bool (*existsPci)(uint32 bus_, uint32 device_, uint32 function_);

  int32 (*read32)(struct class_PciMC *th, uint64 event, uint32 * value);
  int32 (*write32)(struct class_PciMC *th, uint64 event, uint32 value);

  int32 (*read64)(struct class_PciMC *th, uint64 event, uint64 * value);
  int32 (*write64)(struct class_PciMC *th, uint64 event, uint64 value);

  void (*UPciMC)(struct class_PciMC *th);
};

void msr_msr(msr *th, int cpu) ;
void msr_Umsr(msr *th) ;
int msr_read(msr *th, uint64 event, uint64* value);
int msr_write(msr *th, uint64 event, uint64 value);
void PciMC_PciMC(PciMC *th, uint32 bus_, uint32 device_, uint32 function_);
bool PciMC_existsPci(uint32 bus_, uint32 device_, uint32 function_);
int32 PciMC_read32(PciMC *th, uint64 event, uint32 * value);
int32 PciMC_write32(PciMC *th, uint64 event, uint32 value);
int32 PciMC_read64(PciMC * th,uint64 event,uint64 * value);
int32 PciMC_write64(PciMC *th, uint64 event, uint64 value);
void PciMC_UPciMC(PciMC *th);

#endif
