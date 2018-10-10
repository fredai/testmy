
/*
 * Copyright (C) Inspur(Bejing)
 *
 */

#ifndef CPUINFO_H
#define CPUINFO_H

#define MAX_PCI_BUS_NO     4
#define MAX_PCI_IMC_DEVICE  32
#define MAX_NUM_NODES MAX_PCI_BUS_NO


struct cpuinfo;

struct cpuinfo s_cpuinfo;

struct cpuinfo {

  /* Cores here represent both logical core and physical core. */
  unsigned int NumCores;

  unsigned int NumSockets;
  unsigned int ThreadsPerCore;

  char CPUVendor[256];
  unsigned int CPUFamily;
  unsigned int CPUModel;

  //For CPU Performance Monitoring Units.
  unsigned int NumCoreGeneralCounters;
  unsigned int NumCoreFixedCounters;
  unsigned int CoreGeneralCounterWidth;
  unsigned int CoreFixedCounterWidth;
  unsigned int PerformanceMonitorVersion; 
  unsigned int CboxesPerSocket;

  /* pci bus */
  int pci_bus_no[MAX_PCI_BUS_NO];
};
struct mem_bw {
int mem_read_bw;
int mem_write_bw;
int mem_total_bw;
};
struct pcie_bw {
int pcie_read_bw;
int pcie_write_bw;
int pcie_total_bw;
};

#endif
