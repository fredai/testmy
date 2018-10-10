
/*
 * Copyright (C) Inspur(Bejing)
 * 
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sched.h>
#include <math.h>

#include "cpuinfo.h"

#define PROC_PCI_DEVICES_FILE   "/proc/bus/pci/devices"
#define PROC_PCI_DEVICE_PATH    "/proc/bus/pci"


#define PROC_CPUINFO "/proc/cpuinfo"
#define CPUID(eax, ebx, ecx, edx) \
        __asm__ volatile ("cpuid" \
                        :"=a" (eax), \
                        "=b" (ebx), \
                        "=c" (ecx), \
                        "=d" (edx) \
                        :"0" (eax), "2" (ecx))

#if 0
#define CPUID(eax, ebx, ecx, edx) \
         __asm__ __volatile__ ("push %%ebx; \
                                                cpuid; \
                                                mov %%ebx,%1; \
                                                pop %%ebx" \
                                                :"=a"(eax),"=r"(ebx),"=c"(ecx),"=d"(edx) \
                                                :"0"(eax),"2"(ecx))
#endif



inline unsigned int extractbits(unsigned int value, unsigned int mask, unsigned int begin)
{
        unsigned int ret = 0;

        ret = (value >> begin) & mask;
        return ret;
}

int convert_cpu_model(unsigned int display_family, unsigned int display_model, char *model_string, int len)
{
        switch (display_family) {
        case 0x06:
                switch (display_model) {
                case 0x3f:
                        snprintf(model_string, len, "%s", "Haswell-E");
                        break;
                case 0x3a:
                        snprintf(model_string, len, "%s", "Ivy Bridge");
                        break;       
                case 0x3c:
                        snprintf(model_string, len, "%s", "Haswell");
                        break; 
                case 0x3e:
                        snprintf(model_string, len, "%s", "Ivy Bridge-E");
                        break;  
                case 0x2d:
                        snprintf(model_string, len, "%s", "Sandy Bridge-E");
                        break; 
                case 0x4f:
                        snprintf(model_string, len, "%s", "BROADWELL_E5");
                        break; 
		case 0x57:
                        snprintf(model_string, len, "%s", "Knights Landing");
                        break; 
                default:
                        snprintf(model_string, len, "%s", "UNKNOWN");
                        break;
                }
        default:
                break;
        }
        return 0;
}

int get_cpu_model(char *array, int len)
{
    assert(array);
    
    unsigned int eax = 0x01;
    unsigned int ebx = 0;
    unsigned int ecx = 0;
    unsigned int edx = 0;
    unsigned int model = 0;
    unsigned int family = 0;
    unsigned int display_family = 0;
    unsigned int display_model = 0;
    //char model_string[256] = "";

    CPUID(eax, ebx, ecx, edx);
	printf("%#x\n", eax);
    family = extractbits(eax, 0x0f, 8);
    if (0xf == family) {
        display_family = (extractbits(eax, 0x0f, 20) << 4) + family;
    } else {
        display_family = family;
    }
	printf("display_family:%#.2x\n", display_family);
    model = extractbits(eax, 0x0f, 4);
    if ((0x06 == family) || (0x0f == family)) {
        display_model = (extractbits(eax, 0x0f, 16) << 4) + model;
    } else {
        display_model = model;
    }
	printf("cpu_modle:%#x\n", display_model);
    s_cpuinfo.CPUFamily = display_family;
    s_cpuinfo.CPUModel = display_model;
    convert_cpu_model(display_family, display_model, array, len);
	printf("modle_string:%s\n", array);

    return 0;
}    


int get_PMC()
{
    unsigned int eax = 0;
    unsigned int ebx = 0;
    unsigned int ecx = 0;
    unsigned int edx = 0;    

    CPUID(eax,ebx,ecx,edx);

    if (eax >= 0xA) {
        eax = 0xA;
        CPUID(eax,ebx,ecx,edx);
        s_cpuinfo.PerformanceMonitorVersion = extractbits(eax, 0xff, 0);
        s_cpuinfo.NumCoreGeneralCounters  = extractbits(eax, 0xff, 8);
        s_cpuinfo.CoreGeneralCounterWidth  = extractbits(eax, 0xff, 16);

        if (s_cpuinfo.PerformanceMonitorVersion > 1) {
            s_cpuinfo.NumCoreFixedCounters = extractbits(edx, 0x0f, 0);
            s_cpuinfo.CoreFixedCounterWidth = extractbits(eax, 0x0ff, 5);
        }
    }
    return 0;
}

int get_cpu_topology(void)
    {
        cpu_set_t set;
        int i = 0;
        unsigned int eax = 0;
        unsigned int ebx = 0;
        unsigned int ecx = 0;
        unsigned int edx = 0;  
        unsigned int apicId = 0;
        unsigned int level = 0;
        unsigned int curr_offset = 0;
        unsigned int pre_offset = 0;
        unsigned int level_type = 0;
        unsigned int processors = 0;
        unsigned int mask = 0;
        unsigned int package_id = 0;
        unsigned int core_id = 0;
        unsigned int SMT_id = 0;
        unsigned int pre_core_id = 0;

        processors = sysconf(_SC_NPROCESSORS_CONF);
        if (-1 == processors) {
            perror("sysconf:");
            return -1;
        }
    
        /* get "Maximum Input Value for Basic CPUID Information" */
        eax = 0x0;
        CPUID(eax, ebx, ecx, edx);

        /* parsue cpu topology */
        if (eax >= 0x0B) {
        eax = 0x0B;
        ecx = 0;
        CPUID(eax, ebx, ecx, edx);
        if(!ebx) {
            printf("BLeaf do not support extended topology\n");
            return -1;
        }   
        for (i = 0; i < processors; i++) {
            CPU_ZERO(&set); 
            CPU_SET(i, &set);
            sched_setaffinity(0, sizeof(cpu_set_t), &set);
            level_type = 1;
            level = 0;
            pre_offset = 0;
            while (level_type) {
                        eax = 0x0B;
                        ecx = level;
                        CPUID(eax, ebx, ecx, edx);
                        curr_offset = eax & 0XFU;
                        apicId = edx;
                        
                        /* set the mask according to the bits_width, the register has 32-bits */
                        mask = (curr_offset > pre_offset) ? ((uint32)pow(2, curr_offset - pre_offset) - 1) : 0xffffffff;
                        
                        /* Bits 04-00: Number of bits to shift right on x2APIC ID 
                  * to get a unique topology ID of the next level type 
                  */
                        switch ((ecx >> 8) & 0x0ff) {
                                case 0:
                                        package_id = (apicId >> pre_offset) & mask;
                                        if (package_id > s_cpuinfo.NumSockets) {
                                            s_cpuinfo.NumSockets++;
                                        }
                                        break;
                                case 1:
                                        SMT_id = (apicId >> pre_offset) & mask;
                                        if (SMT_id > s_cpuinfo.ThreadsPerCore) {
                                            s_cpuinfo.ThreadsPerCore++;
                                        }
                                        break;
                                case 2:
                                        core_id = (apicId >> pre_offset) & mask;
                                        //if (core_id > s_cpuinfo.NumCores) {
                                        if (core_id > pre_core_id) {
                                            s_cpuinfo.NumCores++;
                                        }
                                        pre_core_id = core_id;
                                        break;
                                default:
                                        printf("know:%d\n", (apicId >> pre_offset) & mask);
                                        break;
                        }

                        level_type = (ecx >> 8) & 0x0ff;
                        level++;
                        pre_offset = curr_offset;
                }
        }
        
        s_cpuinfo.NumSockets++;
        s_cpuinfo.NumCores += s_cpuinfo.NumSockets;
        s_cpuinfo.ThreadsPerCore++;
        
        }
        return 0;
    }

int get_CBo_num(void)
{
    int i = 0;
    int core_no;
    uint64 pre_cboxes = 0;
    uint64 cboxes = 0;
    int ret = -1;
    int msr_fd = -1;
    char msr_file[256] = {0};

    for (i = 0; i < s_cpuinfo.NumSockets; i++) {
        /* check the core fd in the socket */
        /* uncore_no = i * (s_cpuinfo.NumCores / s_cpuinfo.NumSockets / s_cpuinfo.ThreadsPerCore); */
        core_no = i * (s_cpuinfo.NumCores / s_cpuinfo.NumSockets);
        snprintf(msr_file, sizeof(msr_file), "/dev/cpu/%d/msr", core_no);
        //if (-2 != msr_file_fd[core_no]) {
        if (-1!= (msr_fd = open(msr_file, O_RDWR))) {
            ret = pread(msr_fd, &cboxes, sizeof(uint64), 0x0702);
            if (-1 == ret)  {
                perror("core_no fd pread error");
                close(msr_fd);
                return ret;
            }
            if (0 != i && pre_cboxes != cboxes) {
                printf("error: the number of core in different socket is different\n \
                             socket%d.cboxes%llu,  socket%d.cboxes%llu\n", i -1, pre_cboxes, i, cboxes);
                close(msr_fd);
                return -1;
            } 
            pre_cboxes = cboxes;
            printf("socket%d.cboxes%llu, core_no%d\n", i,  (cboxes & 0x01f), core_no);
            close(msr_fd);
        } else {
            printf("%s error: %s\n", msr_file, strerror(errno));
            return -1;
        }
    }
    s_cpuinfo.CboxesPerSocket = (uint32)(cboxes & 0x01f);
    return 0;
} 

int get_cpuinfo(void)
{
    char array[256] = {0};
    
    get_cpu_model(array, sizeof(array));
    if (!strncmp(array, "", sizeof(array))) {
        printf("the platform is surpported\n");
        return -1;
    }
    get_cpu_topology();
    get_PMC();
    get_CBo_num();
    printf("%s:%d:%d:%d:%d, %d: %d-%d, %d-%d\n", \
                array, s_cpuinfo.NumSockets, s_cpuinfo.NumCores, s_cpuinfo.CboxesPerSocket, s_cpuinfo.ThreadsPerCore, \
                s_cpuinfo.PerformanceMonitorVersion, s_cpuinfo.NumCoreGeneralCounters, s_cpuinfo.CoreGeneralCounterWidth, \
                s_cpuinfo.NumCoreFixedCounters, s_cpuinfo.CoreFixedCounterWidth);
    return 0;
}


int isplatform()
{
    return 0;
}

