
/*
 * Copyright (C) Inspur(Beijing)
 * 
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "con_define.h"
#include "framework.h"
#include "common.h"

#include "t_platform_haswell.c"
#include "cpuinfo.c"

int msr_file_fd[100] = {0};
int pci_file_fd[MAX_PCI_BUS_NO][MAX_PCI_IMC_DEVICE];
int micro_module_col_num = 0;
unsigned int max_num_devices = 0;
char **pci_device = NULL;

static uint32 g_cbox_filter_base_addr = 0;
static uint32 g_cbox_ctr_ctl_base_addr = 0;
static uint32 g_cbox_ctl_base_addr = 0;
static uint32 g_cbox_status_base_addr = 0;
static uint32 g_cbox_ctr_base_addr = 0;
static uint32 g_cbox_addr_step = 0;

int  getCBoCtrRegister(uint32 CBox, uint32 counter_id, int msr_fd, uint64 *value)
{
    assert(NULL != value);
    
    uint32 cbox_ctr0_addr = g_cbox_ctr_base_addr + (g_cbox_addr_step * CBox) + counter_id;

    if (-1 == pread(msr_fd, value, sizeof(value), cbox_ctr0_addr)) {
        perror("pread getCBoCtrRegister error");
        return -1;
    }
    printf("cbox_ctr0_addr = %#.4x, value = %llu\n", cbox_ctr0_addr, *value);
    return 0;
    /*
    * uint32 cbox_ctr1_addr = g_cbox_ctr_base_addr + (g_cbox_addr_step * CBox) + 0x01;
    * uint32 cbox_ctr2_addr = g_cbox_ctr_base_addr + (g_cbox_addr_step * CBox) + 0x02;
    * uint32 cbox_ctr3_addr = g_cbox_ctr_base_addr + (g_cbox_addr_step * CBox) + 0x03;
    * printf("cbox_ctr0_addr = %#.4x\n", cbox_ctr0_addr);
    * printf("cbox_ctr1_addr = %#.4x\n", cbox_ctr1_addr);
    * printf("cbox_ctr2_addr = %#.4x\n", cbox_ctr2_addr);
    * printf("cbox_ctr3_addr = %#.4x\n", cbox_ctr3_addr);
    */
}

int setCBoFilter0Register(uint32 CBox, int msr_fd, uint64 filter_value)
{
    uint64 filter0 = filter_value;
    uint32 cbox_filter0_addr = g_cbox_filter_base_addr + (g_cbox_addr_step * CBox);

    if (-1 == pwrite(msr_fd, &filter0, sizeof(uint64), cbox_filter0_addr)) {
        perror("pwrite setCBoFilter0Register error");
        return -1;
    }
    printf("filter0_addr = %#.4x\n", cbox_filter0_addr);
    return 0;
}

int setCBoFilter1Register(uint32 CBox, int msr_fd, uint64 op_code)
{
    uint64 filter1 = op_code;
    uint32 cbox_filter1_addr = g_cbox_filter_base_addr + (g_cbox_addr_step * CBox) + 0x01;

     if (-1 == pwrite(msr_fd, &filter1, sizeof(uint64), cbox_filter1_addr)) {
        perror("pwrite setCBoFilter1Register error");
        return -1;
    }
    printf("filter1_addr = %#.4x\n", cbox_filter1_addr);
    return 0;
}

int setCBoCtrCtlRegister(uint32 CBox, uint32 counter_id, int msr_fd, uint64 ctr_ctl_value)
{
    uint64 value = ctr_ctl_value;
    uint32 cbox_ctr_ctl_addr = g_cbox_ctr_ctl_base_addr + (g_cbox_addr_step * CBox) + counter_id;
    printf("cbox_ctr_ctl_addr = %#.4x\n", cbox_ctr_ctl_addr);
    /*
    * uint32 cbox_ctr_ctl1_addr = g_cbox_ctr_ctl_base_addr + (g_cbox_addr_step * CBox) + 0x01;
    * uint32 cbox_ctr_ctl2_addr = g_cbox_ctr_ctl_base_addr + (g_cbox_addr_step * CBox) + 0x02;
    * uint32 cbox_ctr_ctl3_addr = g_cbox_ctr_ctl_base_addr + (g_cbox_addr_step * CBox) + 0x03;
    * printf("cbox_ctr_ctl0_addr = %#.4x\n", cbox_ctr_ctl0_addr);
    * printf("cbox_ctr_ctl1_addr = %#.4x\n", cbox_ctr_ctl1_addr);
    * printf("cbox_ctr_ctl2_addr = %#.4x\n", cbox_ctr_ctl2_addr);
    * printf("cbox_ctr_ctl3_addr = %#.4x\n", cbox_ctr_ctl3_addr);
    */
     if (-1 == pwrite(msr_fd, &value, sizeof(uint64), cbox_ctr_ctl_addr)) {
        perror("pwrite setCBoCtrCtlRegister error");
        return -1;
    }
    
    return 0;
}

int setCBoCtlRegister(uint32 CBox, int msr_fd, uint64 ctl_value)
{
    uint64 value = (uint64)ctl_value;
    uint32 cbox_ctl_addr = g_cbox_ctl_base_addr + (g_cbox_addr_step * CBox); 
    
    printf("cbox_ctl_addr = %#.4x\n", cbox_ctl_addr);

    if (-1 == pwrite(msr_fd, &value, sizeof(uint64), cbox_ctl_addr)) {
        perror("pwrite setCBoCtlRegister error");
        return -1;
    }
    return 0;
}

int getCBoxStatusRegister(uint32 CBox, int msr_fd, uint64 op_code)
{
    uint32 cbox_status_addr = g_cbox_status_base_addr + (g_cbox_addr_step * CBox); 
    printf("cbox_status_addr = %#.4x\n", cbox_status_addr);
    return 0;
}


#if 1
int pci_init(unsigned int pci_test_device_id)
{
    int bus_id = 0;
    int dev_func= 0;
    int vendor_device_id = 0;
    FILE *fp = NULL;
    char buf[256] = {0};
    int count = 0;
    int i = 0;
    int j = 0;
    char pci_device_path[256] = {0};

    /*is PROC_PCI_DEVICES_FILE? */
    fp = fopen(PROC_PCI_DEVICES_FILE, "r");
    if (NULL == fp) {
        perror("fopen PROC_PCI_DEVICES_FILE");
        return -1;
    }

    while (fgets(buf, sizeof(buf), fp)) {
        if (3 == sscanf(buf, "%2x%2x %8x", &bus_id, &dev_func, &vendor_device_id) && vendor_device_id == pci_test_device_id) {
            if (count < s_cpuinfo.NumSockets) {
                s_cpuinfo.pci_bus_no[count++] = bus_id;
                printf("bus[%d] = %d\n", count -1, s_cpuinfo.pci_bus_no[count - 1]);
            } else {
                printf("NumSockets < uncore buses\n");
                return -1;
            }
        }
    }

    if (count != s_cpuinfo.NumSockets) {
        printf("please check bus and socket: count = %d, s_cpuinfo.NumSockets = %d\n", count , s_cpuinfo.NumSockets);
        return -1;
    }

    for (i = 0; i < count; i++) {
        for (j = 0; j < max_num_devices; j++) {
            sprintf(pci_device_path, "%s/%2x/%s", PROC_PCI_DEVICE_PATH, s_cpuinfo.pci_bus_no[i], pci_device[j]);
            if (!access(pci_device_path, R_OK | W_OK)) {
                pci_file_fd[i][j] = 0;
                printf("access R_OK | W_OK: %s\n", pci_device_path);
            } else {
                pci_file_fd[i][j] = -2;
            }
        }
    }
    return 0;
}
#endif

static struct mod_info *micro_structure_mod_info;


#if 1
void 
MicroStructure_start()
{
        int i = 0;
        int j = 0;
        char msr_file_name[256] = {0};
        char pci_file_name[256] = {0};
        
	/* open the msr files */
	for(i = 0; i < s_cpuinfo.NumCores && i < 100; i++) {
            snprintf(msr_file_name, sizeof(msr_file_name), "/dev/cpu/%d/msr", i);
            msr_file_fd[i] = open(msr_file_name, O_RDWR);
            if (-1 == msr_file_fd[i]) {
                msr_file_fd[i] = -2;
                perror("msr_file_name:");
                return;
            }
	}

	for (i = 0; i < s_cpuinfo.NumSockets; i++) {
                for (j = 0; j < max_num_devices; j++) {
                    sprintf(pci_file_name, "%s/%2x/%s", PROC_PCI_DEVICE_PATH, s_cpuinfo.pci_bus_no[i], pci_device[j]);
                    if (-1 == (pci_file_fd[i][j] = open(pci_file_name, O_RDWR))) {
                        pci_file_fd[i][j] = -2;
                        printf("open O_RDWR: %s\n", pci_file_name);
                    } 
               }
	}
	
#if 0
	cboxes = get_CBo_num();

	for (i = 0; i < cboxes; i++) {
            getCBoCtrRegister(i, 0, 0);
            setCBoFilter1Register(i, 0, 0);
            setCBoFilter0Register(i, 0, 0);
            setCBoCtrCtlRegister(i, 0, 0);
            setCBoCtlRegister(i, 0, 0);
            getCBoxStatusRegister(i, 0, 0);
	}
#endif

	memset(&haswell_counters, 0, sizeof(haswell_counters));

	/* select events and get counters*/
	switch (s_cpuinfo.CPUModel) {
         case 0x3f:
                    haswell_set_cpi_ctrl();
                    haswell_set_exist_mc_box();
                    haswell_set_pcie_event();
                    haswell_get_cpi_counters(&(haswell_counters.cpi_counter));
                    haswell_get_mem_counters(haswell_counters.mc_channelx_counter);
                    break;
         default:
                    break;
	}
	return;
}


void 
MicroStructure_read(struct module *mod)
{
    assert(mod);
    /* assert(mod->col); */
    struct haswell_fixed_cpi curr_fixed_value;
    struct MCChannelCounter curr_mc_counter[MAX_PCI_BUS_NO][max_num_devices];
    struct MCBW mc_bw;
    float cpi = 0.0;

    memset(&curr_fixed_value, 0, sizeof(curr_fixed_value));
    
    switch (s_cpuinfo.CPUModel) {
         case 0x3f:
                    haswell_get_cpi_counters(&curr_fixed_value);
                    haswell_calc_cpi(&curr_fixed_value, &cpi);
                    haswell_get_mem_counters(curr_mc_counter);
                    haswell_cal_mc_bw(curr_mc_counter, &mc_bw);
                    haswell_set_pcie_event();
                    break;
         default:
                    break;
	}

	snprintf ( (mod->info [0]).index_data, LEN_32, "%f", cpi);
	snprintf ( (mod->info [1]).index_data, LEN_32, "%f", mc_bw.read_bw);
	snprintf ( (mod->info [2]).index_data, LEN_32, "%f", mc_bw.write_bw);
	snprintf ( (mod->info [3]).index_data, LEN_32, "%f", mc_bw.total_bw);
	snprintf ( (mod->info [4]).index_data, LEN_32, "%f", haswell_counters.hsx_pcie_bw.read_bw);
	snprintf ( (mod->info [5]).index_data, LEN_32, "%f", haswell_counters.hsx_pcie_bw.write_bw);
}
#endif



int
mod_register(struct module* mod)
{
    assert(mod);
	/* TODO: add decide module is usealbe in current HW and SW environment */
         /* void *MicroStructure_start = NULL;
        * void *MicroStructure_read = NULL; */

        unsigned int pci_test_device_id = 0;
	
	if(-1 == get_cpuinfo()) {
            printf("do not supported!\n");
            return -1;
	}

	switch (s_cpuinfo.CPUModel) {
         case 0x3f:
                micro_structure_mod_info = haswell_mod_info;
                micro_module_col_num = HASWELL_MODULE_COL_NUM;
                pci_test_device_id = HASWELL_PCI_TEST_DEVICE;
                max_num_devices = HASWELL_MAX_NUM_DEVICES;
                pci_device = haswell_pci_Device;
                g_cbox_filter_base_addr = HSX_CBOX_FILTER_BASE_ADDR;
                g_cbox_ctr_ctl_base_addr = HSX_CBOX_CTR_CTL_BASE_ADDR;
                g_cbox_ctl_base_addr = HSX_CBOX_CTL_BASE_ADDR;
                g_cbox_status_base_addr = HSX_CBOX_STATUS_BASE_ADDR;
                g_cbox_ctr_base_addr = HSX_CBOX_CTR_BASE_ADDR;
                g_cbox_addr_step = HSX_CBOX_ADDR_STEP;
                break;
         default:
                printf("please check get_cpuinfo\n");
                return -1;
	}
         pci_init(pci_test_device_id);
	memset(msr_file_fd, -2, sizeof(int));
	
	register_module_fields(mod, micro_structure_mod_info, \
						  micro_module_col_num, MicroStructure_start, MicroStructure_read);
	return 0;
}

