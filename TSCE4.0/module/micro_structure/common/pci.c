
/*
 * Copyright (C) Inspur(Bejing)
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "common.h"
#include "cpuinfo.h"

#define PCI_ROOT_PATH  "/proc/bus/pci"

static char *socket_bus[MAX_NUM_NODES];
static int socket_count = 0;

static int FD[MAX_NUM_NODES][MAX_PCI_IMC_DEVICE];

void
pci_init(uint32 testDevice, uint32 MAX_NUM_DEVICES, char *pci_Device[])
{
    int cntr = 0;
    int j=0;
    int i=0;
    FILE *fptr;
    char buf[1024];
    char filepath[256] = {0};
    uint32 sbus;
    uint32 sdevfn;
    uint32 svend;

    for (j=0; j<MAX_NUM_NODES; j++ )
    {
        socket_bus[j] = "N-A";
        for (i=0; i<MAX_NUM_DEVICES; i++)
        {
            FD[j][i] = 0;
        }
    }

    if ( (fptr = fopen( "/proc/bus/pci/devices", "r")) == NULL )
    {
        fprintf(stderr, "Unable to open /proc/bus/pci/devices. \
                Thus, no support for PCI based Uncore counters.\n");
        return;
    }

    while( fgets(buf, sizeof(buf)-1, fptr) )
    {
        if ( sscanf(buf, "%2x%2x %8x", &sbus, &sdevfn, &svend) == 3 &&
             svend == testDevice )
        {
            socket_bus[cntr] = (char*)malloc(4);
            sprintf(socket_bus[cntr++], "%02x", sbus);
        }
    }
    fclose(fptr);

    if ( cntr == 0 )
    {
        fprintf(stderr, "Uncore not supported on this system\n");
        return;
    }

    socket_count = cntr;

    sprintf(filepath, "%s/%s/%s", PCI_ROOT_PATH, socket_bus[0], pci_Device[j]);

    if (access(filepath,F_OK))
    {
        fprintf(stderr, "INFO\n");
        fprintf(stderr, "       This system has no support for PCI based Uncore counters.\n");
        fprintf(stderr, "       This means you cannot use performance groups as MEM, which require Uncore counters.\n\n");
        return;
    }

        if(geteuid() != 0)
        {
            fprintf(stderr, "WARNING\n");
            fprintf(stderr, "       Direct access to the PCI Cfg Adressspace is only allowed for uid root!\n");
            fprintf(stderr, "       This means you can use performance groups as MEM only as root in direct mode.\n");
            fprintf(stderr, "       Alternatively you might want to look into (sys)daemonmode.\n\n");
        }

        for (j=0; j<socket_count; j++)
        {
            for (i=0; i<MAX_NUM_DEVICES; i++)
            {

                sprintf(filepath, "%s/%s/%s", PCI_ROOT_PATH, socket_bus[j], pci_Device[i]);
                //printf("pci_filepath: %s\n", filepath);

	        FD[j][i] = open(filepath, O_RDWR); 
            	if (FD[j][i] < 0)
            	{
            	        //fprintf(stderr, "Device %s not found, excluded it from device list\n",bdata(filepath));
            		FD[j][i] = -2;
            	}
            }
        }
   
}

void
pci_finalize(uint32 MAX_NUM_DEVICES)
{
    int i = 0;
    int j = 0;
    
    for (j=0; j<socket_count; j++)
    {
        for (i=0; i<MAX_NUM_DEVICES; i++)
        {
            if (FD[j][i] > 0)
            {
                close(FD[j][i]);
            }
        }
    }
    for (j = 0; j < socket_count; j++) {
        if (!strncmp(socket_bus[socket_count], "N-A", strlen("N-A"))) {
            free(socket_bus[socket_count]);
        }
    }
}


uint64
pci_read64(int socketId, uint32 device_index, char *pci_Device[], uint32 reg)
{
    //int socketId = affinity_core2node_lookup[cpu];

    uint64 data = 0;
    if ( FD[socketId][device_index] == -2)
    {
    	//fprintf(stderr, "Accessing non-existent device %s/%s/%s\n",PCI_ROOT_PATH,socket_bus[socketId],pci_Device[device_index]);
    	return data;
    }

    if ( FD[socketId][device_index] > 0 &&
         pread(FD[socketId][device_index], &data, sizeof(data), reg) != sizeof data ) 
    {
        fprintf(stderr, "socketId %d reg %x",socketId, reg);
    }

    return data;

}



void
pci_write32(int socketId, uint32 device_index, char *pci_Device[], uint32 reg, uint32 data)
{
    //int socketId = affinity_core2node_lookup[cpu];

    if ( FD[socketId][device_index] == -2)
    {
    	//fprintf(stderr, "Accessing non-existent device %s/%s/%s\n",PCI_ROOT_PATH,socket_bus[socketId],pci_Device[device_index]);
    	return;
    }

    if ( FD[socketId][device_index] > 0 &&
         pwrite(FD[socketId][device_index], &data, sizeof(data), reg) != sizeof data) 
    {
        fprintf(stderr, "socketId %d reg %x",socketId, reg);
    }

    //    printf("WRITE Device %s cpu %d reg 0x%x data 0x%x \n",bdata(filepath), cpu, reg, data);
}

