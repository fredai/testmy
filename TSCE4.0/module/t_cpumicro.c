
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

int  getCBoCtrRegister(uint32 CBox, uint32 counter_id, uint32 cbox_map_core, uint64 *value);
int setCBoFilter0Register(uint32 CBox, uint32 cbox_map_core, uint64 filter_value);
int setCBoFilter1Register(uint32 CBox, uint32 cbox_map_core, uint64 op_code);
int setCBoCtrCtlRegister(uint32 CBox, uint32 counter_id, uint32 cbox_map_core, uint64 ctr_ctl_value);
int setCBoCtlRegister(uint32 CBox, uint32 cbox_map_core, uint64 ctl_value);
int getCBoxStatusRegister(uint32 CBox, uint32 cbox_map_core, uint64 op_code);

#include "t_platform_haswell.c"
#include "cpuinfo.c"

#include "./micro_structure/ivy_sandy/client.c"

static uint32 g_cbox_filter_base_addr = 0;
static uint32 g_cbox_ctr_ctl_base_addr = 0;
static uint32 g_cbox_ctl_base_addr = 0;
static uint32 g_cbox_status_base_addr = 0;
static uint32 g_cbox_ctr_base_addr = 0;
static uint32 g_cbox_addr_step = 0;

int  getCBoCtrRegister(uint32 CBox, uint32 counter_id, uint32 cbox_map_core, uint64 *value)
{
    assert(NULL != value);
    
    uint32 cbox_ctr0_addr = g_cbox_ctr_base_addr + (g_cbox_addr_step * CBox) + counter_id;

    *value = msrRead(cbox_map_core, cbox_ctr0_addr);
    return 0;
}

int setCBoFilter0Register(uint32 CBox, uint32 cbox_map_core, uint64 filter_value)
{
    uint64 filter0 = filter_value;
    uint32 cbox_filter0_addr = g_cbox_filter_base_addr + (g_cbox_addr_step * CBox);

    msrWrite(cbox_map_core, cbox_filter0_addr, filter0);
    return 0;
}

int setCBoFilter1Register(uint32 CBox, uint32 cbox_map_core, uint64 op_code)
{
    uint64 filter1 = op_code;
    uint32 cbox_filter1_addr = g_cbox_filter_base_addr + (g_cbox_addr_step * CBox) + 0x01;

    msrWrite(cbox_map_core, cbox_filter1_addr, filter1);
    return 0;
}

int setCBoCtrCtlRegister(uint32 CBox, uint32 counter_id, uint32 cbox_map_core, uint64 ctr_ctl_value)
{
    uint64 value = ctr_ctl_value;
    uint32 cbox_ctr_ctl_addr = g_cbox_ctr_ctl_base_addr + (g_cbox_addr_step * CBox) + counter_id;

    msrWrite(cbox_map_core, cbox_ctr_ctl_addr, value);
    return 0;
}

int setCBoCtlRegister(uint32 CBox, uint32 cbox_map_core, uint64 ctl_value)
{
    uint64 value = (uint64)ctl_value;
    uint32 cbox_ctl_addr = g_cbox_ctl_base_addr + (g_cbox_addr_step * CBox); 
    
    msrWrite(cbox_map_core, cbox_ctl_addr, value);
    return 0;
}

int getCBoxStatusRegister(uint32 CBox, uint32 cbox_map_core, uint64 op_code)
{
    uint32 cbox_status_addr = g_cbox_status_base_addr + (g_cbox_addr_step * CBox); 
    printf("cbox_status_addr = %#.4x\n", cbox_status_addr);
    return 0;
}

int
mod_register(struct module* mod)
{
    assert(mod);
	/* TODO: add decide module is usealbe in current HW and SW environment */
         /* void *MicroStructure_start = NULL;
        * void *MicroStructure_read = NULL; */
	
	if(-1 == get_cpuinfo()) {
            printf("do not supported!\n");
            return MODULE_FLAG_NOT_USEABLE;
	}

	switch (s_cpuinfo.CPUModel) {
         case HASWELL_E5:
                msrInit();
                pci_init(HASWELL_PCI_TEST_DEVICE, HASWELL_MAX_NUM_DEVICES, haswell_pci_Device);
                g_cbox_filter_base_addr = HSX_CBOX_FILTER_BASE_ADDR;
                g_cbox_ctr_ctl_base_addr = HSX_CBOX_CTR_CTL_BASE_ADDR;
                g_cbox_ctl_base_addr = HSX_CBOX_CTL_BASE_ADDR;
                g_cbox_status_base_addr = HSX_CBOX_STATUS_BASE_ADDR;
                g_cbox_ctr_base_addr = HSX_CBOX_CTR_BASE_ADDR;
                g_cbox_addr_step = HSX_CBOX_ADDR_STEP;
                
                register_module_fields(mod, haswell_mod_info, HASWELL_MODULE_COL_NUM, HSX_start, HSX_read);
                break;
                
          case BROADWELL_E5:
//          case SANDYBRIDGE_E5:
//          case IVYBRIDGE_E5:
    	  case KNL:
//		printf("*************************==%d\n", s_cpuinfo.CPUModel);
              register_module_fields(mod, IvySandy_mod_info, IvySandy_MODULE_COL_NUM, TeyeInit, IvySandy_read);
              break;
         default:
                printf("please check cpuinfo model, only support 63/62/45\n");
                return MODULE_FLAG_NOT_USEABLE;
	}
//		printf("*************************==%d\n", s_cpuinfo.CPUModel);
	return 0;
}

