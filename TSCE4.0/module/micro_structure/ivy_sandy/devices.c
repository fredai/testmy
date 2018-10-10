/*Inspur (Beijing) Electronic Information Industry Co., Ltd.
  
  Written by LiuYu
  
  File: devices.cpp
  Version: V1.0 alpha
  Update: 2013-01-28
  
  This file include functions to manipulate msr, pci and mem devices.
  Class msr which is used for reading and writing '/dev/cpu/?/msr' 
  devices.
  Class PciMC which is used for reading and writing '/dev/mem' devices.
*/


//#include <iostream>
//#include <iomanip>
//#include <sstream>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "devices.h"

void msr_init(msr *th)
{
    th->msr = msr_msr;
    th->Umsr = msr_Umsr;
    th->read = msr_read;
    th->write = msr_write;
}

//class msr
void msr_msr(msr *th, int cpu) 
{ 
  th->fd = -1;
  th->cpu_id = cpu;
  
  char path[100]="";

  sprintf(path, "/dev/cpu/%d/msr", cpu);

  th->fd = open(path, O_RDWR);

  if (th->fd < 0) {
    printf("Error while opening MSR!\n");
    printf("Please check whether you have the permission to control MSR devices.\n");

    exit(1);
  }
}

void msr_Umsr(msr *th) 
{
  close (th->fd);
}

int msr_read(msr *th, uint64 event, uint64* value)
{
  int byte = pread(th->fd, (void*)value, sizeof(long), event);

  if (byte < 0) {
    printf("Error while reading MSR!\n");
    exit(1);
  }

  return byte;
}

int msr_write(msr *th, uint64 event, uint64 value)
{
  int byte = pwrite(th->fd, (void*)&value, sizeof(long), event);

  if (byte < 0) {
    printf("Error while writing MSR!\n");
    exit(1);
  }

  return byte;
}

//class PciMC

void PciMC_init(PciMC *th)
{
    th->PciMC = PciMC_PciMC;
    th->existsPci = PciMC_existsPci;
    th->UPciMC = PciMC_UPciMC;
    th->read32 = PciMC_read32;
    th->read64 = PciMC_read64;
    th->write32 = PciMC_write32;
    th->write64 = PciMC_write64;
}

void PciMC_PciMC(PciMC *th, uint32 bus_, uint32 device_, uint32 function_)
{
  th->fd = -1;
  th->bus= bus_;
  th->device = device_;
  th->function = function_;
  th->BaseAddr = 0;

  int handle = open("/dev/mem", O_RDWR);

  if (handle < 0) {
    printf("Error while opening device mem!\n");
    printf("Please check whether you have the permission to control MEM devices.\n");

    exit(1);
  }

  th->fd = handle;

  int mcfg_handle = open("/sys/firmware/acpi/tables/MCFG", O_RDONLY);

  if (mcfg_handle < 0) {
    printf("Error while opening MCFG!\n");
    printf("Please check whether you have the permission to control MCFG devices.\n");

    exit(1);
  }

  int32 result = pread(mcfg_handle, (void *)&th->BaseAddr, sizeof(uint64), 44);

  if (result != sizeof(uint64)) {
    close(mcfg_handle);

    printf("Error while reading MCFG!\n");;

    exit(1);
  }

  unsigned char max_bus = 0;

  result = pread(mcfg_handle, (void *)&max_bus, sizeof(unsigned char), 55);

  if (result != sizeof(unsigned char)) {
    close(mcfg_handle);

    printf("Error while reading maximun bus number!\n" );

    exit(1);
  }

  close(mcfg_handle);

  if(th->bus > (unsigned)max_bus) {
    printf("Error: Requested bus number %d is larger than the max bus number %d.\n", th->bus,  (unsigned)max_bus);

    exit(1);
  }

  th->BaseAddr += (th->bus * 1024 * 1024 + th->device * 32 * 1024 + th->function * 4 * 1024);
}

bool PciMC_existsPci(uint32 bus_, uint32 device_, uint32 function_) {
    
  /* jff note in order to "convert c++ to c"
  std::ostringstream path(std::ostringstream::out);
  path << std::hex << "/proc/bus/pci/" << std::setw(2) 
       << std::setfill('0') << bus_ << "/" << std::setw(2) 
       << std::setfill('0') << device_ << "." << function_;
  */
  
  char path[100] = "";

  sprintf(path, "/proc/bus/pci/%.2x/%.2x.%x", bus_, device_, function_);

  int handle = open(path, O_RDWR);

  if (handle < 0) return false;

  close(handle);
  
  return true;
}

int32 PciMC_read32(PciMC *th, uint64 event, uint32 * value) {
  return pread(th->fd, (void *)value, sizeof(uint32), event + th->BaseAddr);
}

int32 PciMC_write32(PciMC *th, uint64 event, uint32 value) {
  return pwrite(th->fd, (const void *)&value, sizeof(uint32), event + th->BaseAddr);
}

int32 PciMC_read64(PciMC *th, uint64 event, uint64 * value) {
  return pread(th->fd, (void *)value, sizeof(uint64), event + th->BaseAddr);
}

int32 PciMC_write64(PciMC *th, uint64 event, uint64 value) {
  return pwrite(th->fd, (const void *)&value, sizeof(uint64), event + th->BaseAddr);
}

void PciMC_UPciMC(PciMC *th) {
  if (th->fd >= 0) 
    close(th->fd);
}
