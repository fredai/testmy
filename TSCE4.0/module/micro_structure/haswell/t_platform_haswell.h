#ifndef MODULE_HSW_H
#define MODULE_HSW_H

/*
 * Copyright (C) Inspur(Bejing)
 * 
 */
#define U_MSR_PMON_GLOBAL_CONFIG_ADDR 0x0702     /* 32 UBox PMON Global Configuration */
#define U_MSR_PMON_GLOBAL_STATUS_ADDR 0x0701     /* 32 UBox PMON Global Status */
#define U_MSR_PMON_GLOBAL_CTL_ADDR        0x0700     /* 32 UBox PMON Global Control */
     
     /* HASWELL_SERVER_SOCKETID_UBOX_DID 0x2f1e */
#define HASWELL_PCI_TEST_DEVICE     0x80862f1e
     
#define IA32_PERF_GLOBAL_CTRL_ADDR                       0x38F
#define IA32_PERF_GLOBAL_STATUS_ADDR                  0x38E
#define IA32_PERF_GLOBAL_OVF_CTRL_ADDR             0x390
#define IA32_FIXED_CTR_CTRL_ADDR                            0x38D
#define IA32_FIXED_CTR0_ADDR                                     0x309
#define IA32_FIXED_CTR1_ADDR                                     0x30A
     
#define MC_CHy_PCI_PMON_BOX_STATUS   0xF8  /* 32 MC Channel y PMON Box-Wide Status */
#define MC_CHy_PCI_PMON_BOX_CTL 0xF4 /* 32 MC Channel y PMON Box-Wide Control */
#define MC_CHy_PCI_PMON_CTL3 0xE4 /* 32 MC Channel y PMON Control for Counter 3 */
#define MC_CHy_PCI_PMON_CTL2 0xE0 /* 32 MC Channel y PMON Control for Counter 2 */
#define MC_CHy_PCI_PMON_CTL1 0xDC /* 32 MC Channel y PMON Control for Counter 1 */
#define MC_CHy_PCI_PMON_CTL0 0xD8 /* 32 MC Channel y PMON Control for Counter 0 */
#define MC_CHy_PCI_PMON_FIXED_CTR 0xD0 /* D4+D0 32x2 MC Channel y PMON Fixed Counter */
#define MC_CHy_PCI_PMON_CTR3 0xB8 /* BC+B8 32x2 MC Channel y PMON Counter 3 */
#define MC_CHy_PCI_PMON_CTR2 0xB0 /* B4+B0 32x2 MC Channel y PMON Counter 2 */
#define MC_CHy_PCI_PMON_CTR1 0xA8 /* AC+A8 32x2 MC Channel y PMON Counter 1 */
#define MC_CHy_PCI_PMON_CTR0 0xA0 /* A4+A0 32x2 MC Channel y PMON Counter 0 */
     
#define MC_CAS_COUNT_EVENT 0x04

/*   Created by Jiaofenfang   */
//#define MC_CAS_COUNT_RD_UMASK  0x03         /* b00000011 */
//#define MC_CAS_COUNT_WR_UMASK 0x0a          /* b00001100 */
//#define MC_CAS_COUNT_ALL_UMASK 0x0f         /* b00001111 */

/* Modified by Daizhenyu  */
#define MC_CAS_COUNT_RD_UMASK ((1UL << 0) + (1UL << 1))        /* b00000011 */
#define MC_CAS_COUNT_WR_UMASK ((1UL << 2) + (1UL << 3))        /* b00001100 */
#define MC_CAS_COUNT_ALL_UMASK ((1UL << 0) + (1UL << 1) + (1UL << 2) + (1UL << 3))         /* b00001111 */
     
#define HSX_CBOX_FILTER_BASE_ADDR        0x0E05
#define HSX_CBOX_CTR_BASE_ADDR              0x0E08
#define HSX_CBOX_CTR_CTL_BASE_ADDR      0x0E01
#define HSX_CBOX_CTL_BASE_ADDR               0x0E00
#define HSX_CBOX_STATUS_BASE_ADDR        0x0E07
#define HSX_CBOX_ADDR_STEP                        0x010
     
#define PCIRDCUR      0x19E            //PCIe read current
#define PRD                 0x187            //Partial Reads (UC)
#define ITOM               0x1C8            //Request Invalidate Line
#define PCINSWR        0x1E5            //PCIe Non-Snoop Write (partial)
#define PRDTID          0x3F
#define ITOMTID        0x3F
     
#define  HSX_PCIE_EVENT_NUM         5
#define HASWELL_MODULE_COL_NUM 6
     static struct mod_info haswell_mod_info[] = {
             {"cpi","\0"},
             {"memBW_read","\0"},
             {"memBW_write","\0"},
             {"memBW_total", "\0"},
             {"pcie_read", "\0"},
             {"pcie_write", "\0"}
     };
     
     typedef enum 
     {
         HASWELL_PCI_IMC0_DEVICE_CH_0 = 0,
         HASWELL_PCI_IMC0_DEVICE_CH_1,
         HASWELL_PCI_IMC0_DEVICE_CH_2,
         HASWELL_PCI_IMC0_DEVICE_CH_3,
         HASWELL_PCI_IMC1_DEVICE_CH_0,
         HASWELL_PCI_IMC1_DEVICE_CH_1,
         HASWELL_PCI_IMC1_DEVICE_CH_2,
         HASWELL_PCI_IMC1_DEVICE_CH_3,
         HASWELL_MAX_NUM_DEVICES
     }HASWELL_iMCDeviceIndex;
     
     char *haswell_pci_Device[HASWELL_MAX_NUM_DEVICES] = {
      "14.0",   /* PCI_IMC0_DEVICE_CH_0 */
      "14.1",   /* PCI_IMC0_DEVICE_CH_1 */
      "15.0",   /* PCI_IMC0_DEVICE_CH_2 */
      "15.1",   /* PCI_IMC0_DEVICE_CH_3 */
      "17.0",   /* PCI_IMC1_DEVICE_CH_0 */
      "17.1",   /* PCI_IMC1_DEVICE_CH_1 */
      "18.0",   /* PCI_IMC1_DEVICE_CH_2 */
      "18.1"   /* PCI_IMC1_DEVICE_CH_3 */
       }; 
     
     struct haswell_fixed_cpi
     {
         /* refer <<64-ia-32-architectures-software-developer-manual-325462>> P2530 Table 18-8 */
         unsigned long long fixed_ctr0;             /* INST_RETIRED.ANY */
         unsigned long long fixed_ctr1;             /* CPU_CLK_UNHALTED.CORE */
     };
     
     struct MCChannelCounter
     {
         uint64 counter0;
         uint64 counter1;
         uint64 counter2;
         uint64 counter3;
     };
     
     struct MCBW
     {
         float read_bw;
         float write_bw;
         float total_bw;
     };

     struct HSXCBoEventPRM
     {
        uint32 ctrl_no;
        uint32 event;
        uint32 umask;
        uint32 tid;
        uint32 op_code;

     };

     struct HSXPciebwEvent
     {
         uint64 PCIeRdCur;
         uint64 PRdwtid;
         uint64 PRd;
         uint64 ItoM;
         uint64 PCIeNSWr;
     };

     struct HSXPCIeBW
     {
        float read_bw;
        float write_bw;
     };
     
     
     struct haswell_counters_value
     {
         struct haswell_fixed_cpi *cpi_counter;
         struct MCChannelCounter mc_channelx_counter[MAX_PCI_BUS_NO][HASWELL_MAX_NUM_DEVICES];
         struct HSXPCIeBW hsx_pcie_bw;
     };
     
     static struct haswell_counters_value haswell_counters;
     
     
     union FixedCtrCtrlRegister {
       struct fixed_fields {
         //CTR0
         unsigned long long os0 : 1;
         unsigned long long usr0 : 1;
         unsigned long long any_thread0 : 1;
         unsigned long long enable_pmi0 : 1;
         //CTR1
         unsigned long long os1 : 1;
         unsigned long long usr1 : 1;
         unsigned long long any_thread1 : 1;
         unsigned long long enable_pmi1 : 1;
         //CTR2
         unsigned long long os2 : 1;
         unsigned long long usr2 : 1;
         unsigned long long any_thread2 : 1;
         unsigned long long enable_pmi2 : 1;
         unsigned long long reserved1 : 52;
       } fields;
       unsigned long long value;
     };
     
     union UMSRPMONGlogbalControl {
         struct ubox_ctl_fields {
             unsigned int pmi_core_sel:18;
             unsigned int rsv1:9;
             unsigned int rsv2:2;
             unsigned int unfrz_all:1;
             unsigned int wk_on_pmi:1;
             unsigned int frz_all:1;
         } fields;
         unsigned int value;
     };
     
     union MCChannelPCIPMONBoxControl {
         struct mcbox_ctl_fields {
             unsigned int rst_ctrl:1;
             unsigned int rst_ctrs:1;
             unsigned int ig1:6;
             unsigned int frz:1;
             unsigned int ig2:7;
             unsigned int rsv:2;
             unsigned int ig:14;
         } fields;
         unsigned int value;
     };
     
     union HSXMCChannelControlRegister {
       struct mc_ctl_fields {
         unsigned int event_select : 8;
         unsigned int umask : 8;
         unsigned int reserved1 : 1; /* Reserved. SW must write to 0 else behavior is undefined */
         unsigned int rst:1;
         unsigned int edge : 1;
         unsigned int ig:1;
         unsigned int ov_en:1;
         unsigned int reserved2 : 1;
         unsigned int enable : 1;
         unsigned int invert : 1;
         unsigned int thresh : 8;
       } fields;
       unsigned int value;
     };
     
     union CBoControlRegister {
         struct {
             unsigned int rst_ctrl:1;
             unsigned int rst_ctrs:1;
             unsigned int rsv:6;
             unsigned int frz:1;
             unsigned int rsv1:7;
             unsigned int rsv2:2;
             unsigned int rsv3:14;
         } fields;
         uint32 value;
     };
     
     union CBoCntControlRegister {
         struct {
             unsigned int ev_sel:8;
             unsigned int umask:8;
             unsigned int rsv1:1;
             unsigned int rst:1;
             unsigned int edge_det:1;
             unsigned int tid_en:1;
             unsigned int rsv2:2;
             unsigned int en:1;
             unsigned int invert:1;
             unsigned int thresh:8;
         } fields;
         uint32 value;
     };
     
     union CBoFilter0Register {
         struct {
             unsigned int tid:6;
             unsigned int rsv1:11;
             unsigned int state:7;
             unsigned int rsv2:8;
         } fields;
         uint32 value;
     };
     
     union CBoFilter1Register {
         struct {
             unsigned int nid:16;
             unsigned int rsv1:4;
             unsigned int opc:9;
             unsigned int rsv2:1;
             unsigned int nc:1;
             unsigned int isoc:1;
             } fields;
         uint32 value;
     };
     
     struct PCIeBWEventHSW {
         // PCI-Express read events from memory
         uint64 PCIeRdCur;
         uint64 PRdwtid;
         uint64 PRd;
         // PCI-Express write events to memory
         uint64 ItoM;
         uint64 PCIeNSWr;
       };

#endif
