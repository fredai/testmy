/* Inspur (Beijing) Electronic Information Industry Co., Ltd.

   Written by LiuYu

   File: types.h
   Version: V3.0.0
   Update: 2014-12-30

   Head file for monitoring CPU microstructure events. 
*/

#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdbool.h>

//typedef unsigned long long uint64;
typedef signed long long int64;
//typedef unsigned int uint32;
typedef signed int int32;

//#define DEBUG

#ifdef DEBUG
#define   printf_debug(fmt,args...) printf(fmt, ##args);
#else
#define   printf_debug(fmt,args...) 
#endif
/*
  CPU Models.
 */
enum supportcpumodels {
  //Xeon SandyBridge-E5 2600 processors
  SANDYBRIDGE_E5 = 0x2D,
  //Xeon IvyBridge-EP 2600 v2 processors
  IVYBRIDGE_E5 = 0x3E,
  //Xeon Haswell-EP 2600 v3 processors
  HASWELL_E5 = 0x3F,
  //BROAD WELL
  BROADWELL_E5 = 0x4F,
  //KNL
  KNL = 0x57
};


/*
  MSR addresses.
  "Intel 64 and IA-32 Architectures Software Developer's Manual"
 */

//global control registers
#define IA32_CR_PERF_GLOBAL_CTRL        (0x38F)
#define IA32_CR_FIXED_CTR_CTRL          (0x38D)

//core general select registers
#define IA32_PERFEVTSEL0_ADDR           (0x186)
#define IA32_PERFEVTSEL1_ADDR           (IA32_PERFEVTSEL0_ADDR + 1)
#define IA32_PERFEVTSEL2_ADDR           (IA32_PERFEVTSEL0_ADDR + 2)
#define IA32_PERFEVTSEL3_ADDR           (IA32_PERFEVTSEL0_ADDR + 3)
//For Processors higher than SandyBridge
//For core only which means that the HT must be off.
#define IA32_PERFEVTSEL4_ADDR           (IA32_PERFEVTSEL0_ADDR + 4)
#define IA32_PERFEVTSEL5_ADDR           (IA32_PERFEVTSEL0_ADDR + 5)
#define IA32_PERFEVTSEL6_ADDR           (IA32_PERFEVTSEL0_ADDR + 6)
#define IA32_PERFEVTSEL7_ADDR           (IA32_PERFEVTSEL0_ADDR + 7)

//core general event counters
#define IA32_PMC0                       (0xC1)
#define IA32_PMC1                       (0xC1 + 1)
#define IA32_PMC2                       (0xC1 + 2)
#define IA32_PMC3                       (0xC1 + 3)
//For Processors higher than SandyBridge
//For core only which means that the HT must be off.
#define IA32_PMC4                       (0xC1 + 4)
#define IA32_PMC5                       (0xC1 + 5)
#define IA32_PMC6                       (0xC1 + 6)
#define IA32_PMC7                       (0xC1 + 7)

//core fixed counters
#define INST_RETIRED_ANY_ADDR           (0x309)
#define CPU_CLK_UNHALTED_THREAD_ADDR    (0x30A)
#define CPU_CLK_UNHALTED_REF_ADDR       (0x30B)

/*
  SandyBridge-EP and IvyBridge-EP Uncore integrated memory 
  controler per-component performance monitor address
  "Intel Xeon Processor E5-2600 Product Family Uncore Performance 
  Monitoring Guide"
  "Ivy Bridge Uncore Performance Monitoring Reference Manual"
  "HSX Uncore Programming Guide_521687_rev0.7"
 */
//MC channel PMON Box-Wide Control Register (32bit)
#define MC_CH_PCI_PMON_BOX_CTL (0x0F4)

//MC channel PMON Control Register for Counter {0 : 3} (32bit)
#define MC_CH_PCI_PMON_CTL3 (0x0E4)
#define MC_CH_PCI_PMON_CTL2 (0x0E0)
#define MC_CH_PCI_PMON_CTL1 (0x0DC)
#define MC_CH_PCI_PMON_CTL0 (0x0D8)

//MC channel PMON Register for Counter {0 : 3} (32 X 2 bit)
#define MC_CH_PCI_PMON_CTR3 (0x0B8)
#define MC_CH_PCI_PMON_CTR2 (0x0B0)
#define MC_CH_PCI_PMON_CTR1 (0x0A8)
#define MC_CH_PCI_PMON_CTR0 (0x0A0)

/*
  For SandyBridge-EP and IvyBridge-EP Processoers
  SandyBridge-EP and IvyBridge-EP Uncore CBo per-component 
  performance monitor register address
  "Intel Xeon Processor E5-2600 Product Family Uncore Performance 
  Monitoring Guide"
  "Ivy Bridge Uncore Performance Monitoring Reference Manual"
*/
//Table 2-16
#define C0_MSR_PMON_CTR0        0x0D16 //CBo 0 PMON Counter 0
#define C0_MSR_PMON_CTL0        0x0D10 //CBo 0 PMON Control for Counter 0
#define C0_MSR_PMON_BOX_FILTER  0x0D14 //CBo 0 PMON Filter
#define C0_MSR_PMON_BOX_FILTER1 0x0D1A //CBo 0 PMON Filter 1 (For IvyBridge-EP)
#define C0_MSR_PMON_BOX_CTL     0x0D04 //CBo 0 PMON Box-Wide Control
#define CBO_MSR_STEP            0x0020 //Address step between different CBo
//Table 2-17 (For SandyBridge-EP and IvyBridge-EP)
//Table 2-14 (For Haswell-EP)
#define CBO_MSR_PMON_BOX_CTL_FRZ_EN (1<<16)
#define CBO_MSR_PMON_BOX_CTL_FRZ (1<<8)
#define CBO_MSR_PMON_BOX_CTL_RST_COUNTERS (1<<1)
/*
  For Haswell-EP Processoers
  "HSX Uncore Programming Guide_521687_rev0.7"
  Table 2-13
*/
#define HSW_C0_MSR_PMON_CTR0        0x0E08 //CBo 0 PMON Counter 0
#define HSW_C0_MSR_PMON_CTL0        0x0E01 //CBo 0 PMON Control for Counter 0
#define HSW_C0_MSR_PMON_BOX_FILTER  0x0E05 //CBo 0 PMON Filter 0
#define HSW_C0_MSR_PMON_BOX_FILTER1 0x0E06 //CBo 0 PMON Filter 1
#define HSW_C0_MSR_PMON_BOX_CTL     0x0E00 //CBo 0 PMON Box-Wide Control
#define HSW_CBO_MSR_STEP            0x0010 //Address step between different CBo

#define UNCORE_PMON_BOX_CTL_VALID_BITS_MASK ((1<<17)-1)

//Table 2-19 (For SandyBridge-EP and IvyBridge-EP)
//Table 2-16 (For Haswell-EP)
#define CBO_MSR_PMON_CTL_EN (1<<22)
#define CBO_MSR_PMON_CTL_EVENT(x) (x<<0)
#define CBO_MSR_PMON_CTL_UMASK(x) (x<<8)
#define CBO_MSR_PMON_CTL_TID_EN (1<<19)

//Table 2-22
#define SANDYBRIDGE_E5_CBO_MSR_PMON_BOX_FILTER_OPCODE(x) (x<<23)
#define IVYBRIDGE_E5_CBO_MSR_PMON_BOX_FILTER1_OPCODE(x) (x<<20)
//Table 2-20
#define HASWELL_E5_CBO_MSR_PMON_BOX_FILTER1_OPCODE(x) (x<<20)

//Table 2-23
//Opcode for CX_MSR_PMON_BOX_FILTER.opc
#define PCIPRD        0x195            //PCIe UC Read
#define PCIRDCUR      0x19E            //PCIe read current
#define PCINSRD       0x1E4            //PCIe Non-Snoop Read
#define PCIWILF       0x194            //PCIe Write (non-allocating)
#define PCIITOM       0x19C            //PCIe Write (allocating)
#define PCINSWR       0x1E5            //PCIe Non-Snoop Write (partial)
#define PCINSWRF      0x1E6            //PCIe Non-Snoop Read (full)
//Formore informance please refer to
//"HSX Uncore Programming Guide_521687_rev0.7" Table 2-20
#define RFO           0x180            //Deman Data RFO
#define CRD           0x181            //Demand Code Read
#define DRD           0x182            //Demand Data Read
#define ITOM          0x1C8            //Request Invalidate Line
#define PRD           0x187            //Partial Reads (UC)
#define WIL           0x18F            //Write Invalidate Line
//For more informance please refer to 
//"HSX Uncore Programming Guide_521687_rev0.7" Table 2-18
//Filter the shared opc from CPU to PCIe
#define PRDTID        0x3F
#define ITOMTID       0x3F

/*
  PMU events and umasks
  "Intel 64 and IA-32 Architectures Software Developer's Manual"
 */

//Floating point event
#define FP_COMP_OPS_EXE      0x10
//For SandyBridge
#define SIMD_FP_256          0x11

//Floating point umask
#define X87                  0x01
#define SSE_FP_PACKED_DOUBLE 0x10
#define SSE_SCALAR_DOUBLE    0x80
#define SSE_FP_PACKED_SINGLE 0x40
#define SSE_SCALAR_SINGLE    0x20
//For SandyBridge
#define AVX_PACKED_DOUBLE    0x02
#define AVX_PACKED_SINGLE    0x01

/*
  SandyBridge-EP Uncore integrated memory controler events and umasks
  "Intel Xeon Processor E5-2600 Product Family Uncore Performance 
  Monitoring Guide"
 */
//DRAM channel access signal Event
#define CAS_COUNT            0x04

//DRAM CAS read and write umask
#define CAS_COUNT_RD         ((1UL << 0) + (1UL << 1))
#define CAS_COUNT_WR         ((1UL << 2) + (1UL << 3))



/*
  General event select register
  "Intel 64 and IA-32 Architectures Software Developer's Manual"
 */

union GeneralEventSelectRegister {
  struct {
    uint64 event_select : 8;
    uint64 umask : 8;
    uint64 usr : 1;
    uint64 os : 1;
    uint64 edge : 1;
    uint64 pin_control : 1;
    uint64 apic_int : 1;
    uint64 any_thread : 1;
    uint64 enable : 1;
    uint64 invert : 1;
    uint64 cmask : 8;
    uint64 reserved1 : 32;
  } fields;
  uint64 value;
};

/*
  Fixed event control register
  "Intel 64 and IA-32 Architectures Software Developer's Manual"
  Layout of IA32_FIXED_CTR_CTRL MSR
  For Performance Monitoring Version 3
 */

union FixedEventControlRegister {
  struct {
    //CTR0
    uint64 os0 : 1;
    uint64 usr0 : 1;
    uint64 any_thread0 : 1;
    uint64 enable_pmi0 : 1;
    //CTR1
    uint64 os1 : 1;
    uint64 usr1 : 1;
    uint64 any_thread1 : 1;
    uint64 enable_pmi1 : 1;
    //CTR2
    uint64 os2 : 1;
    uint64 usr2 : 1;
    uint64 any_thread2 : 1;
    uint64 enable_pmi2 : 1;
    uint64 reserved1 : 52;
  } fields;
  uint64 value;
};

/*
  SandyBridge-EP Uncore integrated memory controler channel PMON register 
  control register (32bit)
  "Intel Xeon Processor E5-2600 Product Family Uncore Performance 
  Monitoring Guide"
 */

union MCChannelControlRegister {
  struct {
    uint32 event_select : 8;
    uint32 umask : 8;
    uint32 reserved1 : 2;
    uint32 edge : 1;
    uint32 reserved2 : 1;
    uint32 reserved3 : 2;
    uint32 enable : 1;
    uint32 invert : 1;
    uint32 thresh : 8;
  } fields;
  uint32 value;
};

/*
  Struct EventAndUmask contains the Event and Umask for event
  select registers.
 */
struct EventAndUmask {
  uint32 event;
  uint32 umask;
};

#endif
