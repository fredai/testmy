
/*
 * Copyright (C) Inspur(Bejing)
 *
 */

//#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>

#include "cpuinfo.h"
#include "t_platform_haswell.h"
#include "msr.h"
#include "pci.h"

extern struct cpuinfo s_cpuinfo;

static unsigned long long hsx_milliseconds = 0;
static sem_t hsx_sem_pcie_get;
static sem_t hsx_sem_pcie_put;

int haswell_set_cbo_event(struct HSXCBoEventPRM cbo_event_prm)
{
    int i = 0;
    int j = 0;
    union CBoControlRegister cbox_ctrl;
    union CBoFilter0Register   filter0;
    union CBoFilter1Register   filter1;
    union CBoCntControlRegister cbox_cnt_ctl;
    uint32 cores_per_socket = s_cpuinfo.NumCores / s_cpuinfo.NumSockets / s_cpuinfo.ThreadsPerCore;

    for (i = 0; i < s_cpuinfo.NumSockets; i++) {
        for (j = 0; j < cores_per_socket; j++) {
            /* set cbox ctl register in order to frozen*/
            cbox_ctrl.value = 0;
            cbox_ctrl.fields.frz = 1;
            setCBoCtlRegister(j, i*cores_per_socket, (uint64)cbox_ctrl.value);

            /* set filter0 */
            if (0 != cbo_event_prm.tid) {
                filter0.value = 0;
                filter0.fields.tid = cbo_event_prm.tid;
                setCBoFilter0Register(j, i*cores_per_socket, (uint64)filter0.value);
            }
            /* set filter1 */
            filter1.value = 0;
            filter1.fields.opc = cbo_event_prm.op_code;
            setCBoFilter1Register(j, i*cores_per_socket, (uint64)filter1.value);
            
            /* enable couters in the cbox */
            cbox_cnt_ctl.value = 0;
            cbox_cnt_ctl.fields.en =1;
            setCBoCtrCtlRegister(j, cbo_event_prm.ctrl_no, i*cores_per_socket, (uint64)cbox_cnt_ctl.value);
            cbox_cnt_ctl.fields.ev_sel = cbo_event_prm.event;
            cbox_cnt_ctl.fields.umask = cbo_event_prm.umask;
            cbox_cnt_ctl.fields.rst =1;
            /* if tid != 0 tid_enable */
            cbox_cnt_ctl.fields.tid_en = (cbo_event_prm.tid ? 1 : 0);
            setCBoCtrCtlRegister(j, cbo_event_prm.ctrl_no, i*cores_per_socket, (uint64)cbox_cnt_ctl.value);

            /* reset and unfrozen counters */
            cbox_ctrl.value =0;
            cbox_ctrl.fields.rst_ctrs = 1;
            cbox_ctrl.fields.frz = 0;
            setCBoCtlRegister(j, i*cores_per_socket, (uint64)cbox_ctrl.value);
        }
    }
    return 0;
}

int haswell_get_cbo_counter(uint32 ctr_id, uint64 *value)
{
    int i = 0;
    int j = 0;
    int cores_per_socket = s_cpuinfo.NumCores / s_cpuinfo.NumSockets / s_cpuinfo.ThreadsPerCore;
    uint64 tmp64 = 0;
    uint64 sum = 0;

    for (i = 0; i < s_cpuinfo.NumSockets; i++) {
        for (j = 0; j < cores_per_socket; j++) {
                getCBoCtrRegister(j, ctr_id, i*cores_per_socket , &tmp64);
                sum += tmp64;
        }
    }
    *value = sum;
    return 0;
}

void *haswell_set_pcie_event(void *argv)
{
    uint32 timeval = 200 * 1000;
    uint32 ctr_id = 0;
    struct HSXPciebwEvent counters;
    struct HSXCBoEventPRM cbo_event_prm;

    /* TOR_INSERTS.OPCODE */
    cbo_event_prm.ctrl_no = 0x0;
    cbo_event_prm.event = 0x35;
    cbo_event_prm.umask = 0x01;

    for (; ;) {
        /* 
        * PCIE_DATA_BYTES:
        * (TOR_INSERTS.OPCODE
        * with:Cn_MSR_PMON_BOX_FILTER1.opc=0x194 +
        * TOR_INSERTS.OPCODE
        * with:{Cn_MSR_PMON_BOX_FILTER0.tid=0x3F,
        * Cn_MSR_PMON_BOX_FILTER1.opc=0x1C8}) * 64
        */
        cbo_event_prm.tid = 0;
        cbo_event_prm.op_code = PCIRDCUR;
        haswell_set_cbo_event(cbo_event_prm);
        usleep(timeval);
        haswell_get_cbo_counter(ctr_id, &(counters.PCIeRdCur));

        cbo_event_prm.tid = 0x3F;
        cbo_event_prm.op_code = PRD;
        haswell_set_cbo_event(cbo_event_prm);
        usleep(timeval);
        haswell_get_cbo_counter(ctr_id, &(counters.PRdwtid));

        /* 
        * PARTIAL_PCI_WRITES:
        * TOR_INSERTS.OPCODE
        * with:Cn_MSR_PMON_BOX_FILTER1.opc=0x1E5
        */
        cbo_event_prm.tid = 0;
        cbo_event_prm.op_code = PRD;
        haswell_set_cbo_event(cbo_event_prm);
        usleep(timeval);
        haswell_get_cbo_counter(ctr_id, &(counters.PRd));

        cbo_event_prm.tid = ITOMTID;
        cbo_event_prm.op_code= ITOM;
        haswell_set_cbo_event(cbo_event_prm);
        usleep(timeval);
        haswell_get_cbo_counter(ctr_id, &(counters.ItoM));

        cbo_event_prm.tid = 0;
        cbo_event_prm.op_code = PCINSWR;
        haswell_set_cbo_event(cbo_event_prm);
        usleep(timeval);
        haswell_get_cbo_counter(ctr_id, &(counters.PCIeNSWr));

        sem_wait(&hsx_sem_pcie_put);
        haswell_counters.hsx_pcie_bw.read_bw =  (counters.PCIeRdCur + counters.PRdwtid 
    			  + counters.PRd) * 64 * HSX_PCIE_EVENT_NUM * 1.0 / MB;
        haswell_counters.hsx_pcie_bw.write_bw =  (counters.ItoM + counters.PCIeNSWr) * 64 * HSX_PCIE_EVENT_NUM * 1.0 / MB;
        sem_post(&hsx_sem_pcie_get);
    }
    //return;
}

int haswell_set_mem_event()
{
    union MCChannelPCIPMONBoxControl mc_channel_box_ctl; /*MC_CHy_PCI_PMON_BOX_CTL*/
    union HSXMCChannelControlRegister mc_box_ctr_ctl0; /* CAS_COUNT.RD control*/
    union HSXMCChannelControlRegister mc_box_ctr_ctl1; /* CAS_COUNT.WR control */
    int i = 0;
    int j = 0;

    for (i = 0; i < s_cpuinfo.NumSockets; i++) {
        for (j = 0; j < HASWELL_MAX_NUM_DEVICES; j++) {

            /* bit-8, If set to 1 the counters in this box will be frozen */
            mc_channel_box_ctl.value = 0;
            mc_channel_box_ctl.fields.frz = 1;
            pci_write32(i, j, haswell_pci_Device, MC_CHy_PCI_PMON_BOX_CTL, mc_channel_box_ctl.value);
            
            /* ctl0 is set mem_read_bw */
             mc_box_ctr_ctl0.value = 0;
             mc_box_ctr_ctl0.fields.enable = 1;
             mc_box_ctr_ctl0.fields.event_select = MC_CAS_COUNT_EVENT;
             mc_box_ctr_ctl0.fields.umask = MC_CAS_COUNT_RD_UMASK;
             /* mc_box_ctr_ctl0.fields.ov_en = 1;
             * mc_box_ctr_ctl0.fields.rst = 1;
             * mc_box_ctr_ctl0.fields.reserved1 = 0x0;
             * mc_box_ctr_ctl0.fields.reserved2 = 0x0;
             */
             
             /* ctl1 is set mem_write_bw */
             mc_box_ctr_ctl1.value = 0;
             mc_box_ctr_ctl1.fields.enable = 1;
             mc_box_ctr_ctl1.fields.event_select = MC_CAS_COUNT_EVENT;
             mc_box_ctr_ctl1.fields.umask = MC_CAS_COUNT_WR_UMASK;
             /*mc_box_ctr_ctl1.fields.ov_en = 1;
             *mc_box_ctr_ctl1.fields.rst = 1;
             *mc_box_ctr_ctl1.fields.reserved1 = 0x0;
             *mc_box_ctr_ctl1.fields.reserved2 = 0x0;
             */
            pci_write32(i, j, haswell_pci_Device, MC_CHy_PCI_PMON_CTL0, mc_box_ctr_ctl0.value);
            pci_write32(i, j, haswell_pci_Device, MC_CHy_PCI_PMON_CTL1, mc_box_ctr_ctl1.value);
             
            /* reset counters in the box 
            * to ensure no stale value have been acquired from previous session.
            */ 
            mc_channel_box_ctl.fields.rst_ctrs = 1;
            /* unfrozen MC_Box */
            mc_channel_box_ctl.fields.frz = 0;
            pci_write32(i, j, haswell_pci_Device, MC_CHy_PCI_PMON_BOX_CTL, mc_channel_box_ctl.value);
        }
    }
    return 0;
}

int haswell_set_cpi_ctrl(void)
{
    union FixedCtrCtrlRegister fixed_ctr_ctrl;
    uint64 global_ctr_ctrl = 0;
    int i = 0;
    
    for (i = 0; i < s_cpuinfo.NumCores; i++) {
        /* set global disable */
        msrWrite(i, IA32_PERF_GLOBAL_CTRL_ADDR, global_ctr_ctrl);
        if (msrRead(i, IA32_PERF_GLOBAL_CTRL_ADDR) != global_ctr_ctrl)
        {
            printf("disable write !=read,  IA32_FIXED_CTR_CTRL_ADDR\n");
            return -1;
        }
        
        /* set the IA32_FIXED_CTR_CTRL to enable the IA32_FIXED_CRT0 and the IA32_FIXED_CTR1 
       */
        fixed_ctr_ctrl.value = 0;
        
        fixed_ctr_ctrl.fields.os0 = 1;
        fixed_ctr_ctrl.fields.usr0 = 1;
        //fixed_ctr_ctrl.fields.any_thread0 = (s_cpuinfo.PerformanceMonitorVersion >= 3 && s_cpuinfo.ThreadsPerCore > 1) ? 1 : 0;
        fixed_ctr_ctrl.fields.any_thread0 = 0;
        fixed_ctr_ctrl.fields.enable_pmi0 = 0;

        fixed_ctr_ctrl.fields.os1 = 1;
        fixed_ctr_ctrl.fields.usr1 = 1;
        fixed_ctr_ctrl.fields.any_thread1 = (s_cpuinfo.PerformanceMonitorVersion >= 3 && s_cpuinfo.ThreadsPerCore > 1) ? 1 : 0;
        fixed_ctr_ctrl.fields.enable_pmi1 = 0;

        msrWrite(i, IA32_FIXED_CTR_CTRL_ADDR , fixed_ctr_ctrl.value);
        if (fixed_ctr_ctrl.value != msrRead(i, IA32_FIXED_CTR_CTRL_ADDR)) {
            printf("write !=read,  IA32_FIXED_CTR_CTRL_ADDR\n");
            return -1;
        }

        /* set global enable */
        global_ctr_ctrl = 1ULL << 32 | 1ULL << 33;
        msrWrite(i, IA32_PERF_GLOBAL_CTRL_ADDR, global_ctr_ctrl);
        if (global_ctr_ctrl != msrRead(i, IA32_PERF_GLOBAL_CTRL_ADDR)) {
            printf("enable write !=read,   IA32_PERF_GLOBAL_CTRL_ADDR\n");
            return -1;
        }
    }
    
    return 0;
}

int haswell_get_mem_counters(struct MCChannelCounter counters[][HASWELL_MAX_NUM_DEVICES])
{
    int i = 0;
    int j =0;
    union MCChannelPCIPMONBoxControl mc_channel_box_ctl;

    for (i = 0; i < s_cpuinfo.NumSockets; i++) {
        for (j = 0; j < HASWELL_MAX_NUM_DEVICES; j++) {
            /* bit-8, If set to 1 the counters in this box will be frozen */
            mc_channel_box_ctl.value = 0;
            mc_channel_box_ctl.fields.frz = 1;
            pci_write32(i, j, haswell_pci_Device, MC_CHy_PCI_PMON_BOX_CTL, mc_channel_box_ctl.value);
            
            counters[i][j].counter0 = pci_read64(i, j, haswell_pci_Device, MC_CHy_PCI_PMON_CTR0);
            counters[i][j].counter1 = pci_read64(i, j, haswell_pci_Device, MC_CHy_PCI_PMON_CTR1);
            
            /* bit-8, If set to 0 the counters in this box will be unfrozen */
            mc_channel_box_ctl.fields.frz = 0;
            pci_write32(i, j, haswell_pci_Device, MC_CHy_PCI_PMON_BOX_CTL, mc_channel_box_ctl.value);
        }
    }
    
    return 0;
}

int haswell_cal_mc_bw(struct MCChannelCounter curr_counters[][HASWELL_MAX_NUM_DEVICES], struct MCBW *mc_bw)
{
    int i = 0;
    int j = 0;

    uint64 mc_read_count = 0;
    uint64 mc_write_count = 0;

    for (i = 0; i < s_cpuinfo.NumSockets; i++) {
        for (j = 0; j < HASWELL_MAX_NUM_DEVICES; j++) {
            mc_read_count += (curr_counters[i][j].counter0 >= haswell_counters.mc_channelx_counter[i][j].counter0) ? \
                                                (curr_counters[i][j].counter0 - haswell_counters.mc_channelx_counter[i][j].counter0) : \
                                                  curr_counters[i][j].counter0;
                                                  
            mc_write_count += (curr_counters[i][j].counter1 >= haswell_counters.mc_channelx_counter[i][j].counter1) ? \
                                                (curr_counters[i][j].counter1 - haswell_counters.mc_channelx_counter[i][j].counter1) : \
                                                    curr_counters[i][j].counter1;
            
            haswell_counters.mc_channelx_counter[i][j].counter0 = curr_counters[i][j].counter0;
            haswell_counters.mc_channelx_counter[i][j].counter1 = curr_counters[i][j].counter1;
        }
    }

    mc_bw->read_bw = mc_read_count * 64 * 1.0 / GB;
    mc_bw->write_bw = mc_write_count * 64 * 1.0 / GB;
    mc_bw->total_bw = mc_bw->read_bw + mc_bw->write_bw;
    return 0;
}

int haswell_get_cpi_counters(struct haswell_fixed_cpi *cpi_fixed_counter)
{
    int i = 0;

    for (i = 0; i < s_cpuinfo.NumCores; i++) {
        cpi_fixed_counter[i].fixed_ctr0 = msrRead(i, IA32_FIXED_CTR0_ADDR);
        cpi_fixed_counter[i].fixed_ctr1 = msrRead(i, IA32_FIXED_CTR1_ADDR);

        //printf("%llu, %llu\n", cpi_fixed_counter[i].fixed_ctr0, cpi_fixed_counter[i].fixed_ctr1);
    }
    return 0;
}

int haswell_calc_cpi(struct haswell_fixed_cpi *cpi_fixed_counter, float *cpi)
{
    assert(cpi_fixed_counter);

    int i = 0;
    float sum_cpi = 0.0;  
    unsigned long long instructions = 0;
    unsigned long long cycles = 0;

    for (i = 0; i < s_cpuinfo.NumCores; i++) {
        instructions = (cpi_fixed_counter[i].fixed_ctr0 >= haswell_counters.cpi_counter[i].fixed_ctr0) ? \
                                     (cpi_fixed_counter[i].fixed_ctr0 - haswell_counters.cpi_counter[i].fixed_ctr0) : \
                                     cpi_fixed_counter[i].fixed_ctr0;
                                     
        if (instructions != 0) {
            cycles = (cpi_fixed_counter[i].fixed_ctr1 >= haswell_counters.cpi_counter[i].fixed_ctr1) ? \
                                (cpi_fixed_counter[i].fixed_ctr1 - haswell_counters.cpi_counter[i].fixed_ctr1) : \
                                cpi_fixed_counter[i].fixed_ctr1;
                                
            sum_cpi += (cycles * 1.0) / instructions;
        }

        haswell_counters.cpi_counter[i].fixed_ctr0 = cpi_fixed_counter[i].fixed_ctr0;
        haswell_counters.cpi_counter[i].fixed_ctr1 = cpi_fixed_counter[i].fixed_ctr1;
    }
    if (s_cpuinfo.NumCores != 0)
        *cpi = sum_cpi / s_cpuinfo.NumCores;
    else
        printf("NumCores = %d, cpi error\n", s_cpuinfo.NumCores);
    
    return 0;
}

void HSX_stop();

void 
HSX_start(void)
{
    
    HSX_stop();

    pthread_t tid;
    pthread_attr_t attr;
    int err = -1;
    
    haswell_counters.cpi_counter = (struct haswell_fixed_cpi *)malloc(sizeof(struct haswell_fixed_cpi) * s_cpuinfo.NumCores);
    haswell_set_cpi_ctrl();
    haswell_set_mem_event();
    
    //haswell_set_pcie_event();
    err = sem_init(&hsx_sem_pcie_get, 0, 0);
    if (err != 0) {
        printf("sem_init error: %s\n", strerror(errno));
        return;
    }
    err = sem_init(&hsx_sem_pcie_put, 0, 1);
    if (err != 0) {
        printf("sem_init error: %s\n", strerror(errno));
        return;
    }

    err = pthread_attr_init(&attr);
    if (err != 0) {
        printf("pthread_attr_init error: %s\n", strerror(errno));
        return;
    }
    
    err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (0 == err) {
        err = pthread_create(&tid, &attr, haswell_set_pcie_event, NULL);
    }
    pthread_attr_destroy(&attr);

    hsx_milliseconds = get_current_millisecond();
    
    haswell_get_cpi_counters(haswell_counters.cpi_counter);
    haswell_get_mem_counters(haswell_counters.mc_channelx_counter);

    return;
}


void 
HSX_read(struct module *mod)
{
    assert(mod);
    
    struct haswell_fixed_cpi curr_fixed_value[s_cpuinfo.NumCores];
    struct MCChannelCounter curr_mc_counter[MAX_PCI_BUS_NO][HASWELL_MAX_NUM_DEVICES];
    struct MCBW mc_bw;
    float cpi = 0.0;
    double seconds = 0.0;
    unsigned long long milliseconds = 0;
    unsigned long long curr_milliseconds = get_current_millisecond();

    haswell_get_cpi_counters(curr_fixed_value);
    haswell_calc_cpi(curr_fixed_value, &cpi);
    haswell_get_mem_counters(curr_mc_counter);
    haswell_cal_mc_bw(curr_mc_counter, &mc_bw);
    //haswell_set_pcie_event();

    milliseconds = (curr_milliseconds >= hsx_milliseconds) ? (curr_milliseconds - hsx_milliseconds) : curr_milliseconds;
    hsx_milliseconds = curr_milliseconds;
    if (milliseconds == 0) {
        printf("hsx_read milliseconds = %llu\n", milliseconds);
        snprintf ( (mod->info [1]).index_data, LEN_32, "%.2f", 0.0);
        snprintf ( (mod->info [2]).index_data, LEN_32, "%.2f", 0.0);
        snprintf ( (mod->info [3]).index_data, LEN_32, "%.2f", 0.0);
    } else {
        seconds = milliseconds / 1000.0;
        snprintf ( (mod->info [1]).index_data, LEN_32, "%.2f", mc_bw.read_bw / seconds);
        snprintf ( (mod->info [2]).index_data, LEN_32, "%.2f", mc_bw.write_bw / seconds);
        snprintf ( (mod->info [3]).index_data, LEN_32, "%.2f", mc_bw.total_bw / seconds);
    }
    snprintf ( (mod->info [0]).index_data, LEN_32, "%.2f", cpi);
    sem_wait(&hsx_sem_pcie_get);
    snprintf ( (mod->info [4]).index_data, LEN_32, "%.2f", haswell_counters.hsx_pcie_bw.read_bw);
    snprintf ( (mod->info [5]).index_data, LEN_32, "%.2f", haswell_counters.hsx_pcie_bw.write_bw);
    sem_post(&hsx_sem_pcie_put);

    return;
}

void 
HSX_stop()
{
    union MCChannelPCIPMONBoxControl mc_channel_box_ctl; /*MC_CHy_PCI_PMON_BOX_CTL*/
    union CBoControlRegister cbox_ctrl;
    int i = 0;
    int j = 0;
    
    /* stop cpi counters */
    for (i = 0; i < s_cpuinfo.NumCores; i++) {
        msrWrite(i, IA32_PERF_GLOBAL_CTRL_ADDR, 0);
        msrWrite(i, IA32_FIXED_CTR_CTRL_ADDR , 0);
    }
    
    /* stop mem counters */
    for (i = 0; i < s_cpuinfo.NumSockets; i++) {
        for (j = 0; j < HASWELL_MAX_NUM_DEVICES; j++) {
            mc_channel_box_ctl.value = 0;
            mc_channel_box_ctl.fields.frz = 1;
            pci_write32(i, j, haswell_pci_Device, MC_CHy_PCI_PMON_BOX_CTL, mc_channel_box_ctl.value);
            pci_write32(i, j, haswell_pci_Device, MC_CHy_PCI_PMON_CTL0, 0);
            pci_write32(i, j, haswell_pci_Device, MC_CHy_PCI_PMON_CTL1, 0);
        }
    }

    /* stop pcie counters */
    uint32 cores_per_socket = s_cpuinfo.NumCores / s_cpuinfo.NumSockets / s_cpuinfo.ThreadsPerCore;
    for (i = 0; i < s_cpuinfo.NumSockets; i++) {
        for (j = 0; j < cores_per_socket; j++) {
            cbox_ctrl.value = 0;
            cbox_ctrl.fields.frz = 1;
            setCBoCtlRegister(j, i*cores_per_socket, (uint64)cbox_ctrl.value);
            setCBoCtrCtlRegister(j, 0, i*cores_per_socket, 0);
        }
    }
    return;
}

