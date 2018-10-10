
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

static int *FD = NULL;

void
msrInit(void)
{
        int  fd;
        uint32 i;
        char msr_file_name[64] = {0};
        
        sprintf(msr_file_name,"/dev/cpu/0/msr");
        
        fd = open(msr_file_name, O_RDWR);

        if (fd < 0)
        {
            fprintf(stderr, "ERROR\n");
            fprintf(stderr, "rdmsr: failed to open '%s': %s!\n",
                    msr_file_name , strerror(errno));
            fprintf(stderr, "       Please check if the msr module \
                    is loaded and the device file has correct permissions.\n");
            fprintf(stderr, "       Alternatively you might want to \
                    look into (sys)daemonmode.\n\n");
            exit(127);
        }

        close(fd);

        FD = (int *)malloc(sizeof(int) * s_cpuinfo.NumCores);

        /* NOTICE: This assumes consecutive processor Ids! */
        for ( i=0; i < s_cpuinfo.NumCores; i++ )
        {
            snprintf(msr_file_name, sizeof(msr_file_name), "/dev/cpu/%d/msr",i);

            FD[i] = open(msr_file_name, O_RDWR);

            if ( FD[i] < 0 )
            {
                printf("Error:open %s--%s\n", msr_file_name, strerror(errno));
            }
        }
}

void
msrFinalize(void)
{
    uint32 i = 0;
    
     if (NULL != FD) {
     
        for ( i=0; i < s_cpuinfo.NumCores; i++ )
        {
            close(FD[i]);
        }
   
        free(FD);
    }
}


uint64 
msrRead( const int cpu, uint32 reg)
{
        uint64 data = 0;

        if ( pread(FD[cpu], &data, sizeof(data), reg) != sizeof(data) )
        {
            printf("Error: cpu %d reg %x",cpu, reg);
        }

        return data;
}

void
msrWrite( const int cpu, uint32 reg, uint64 data)
{
        if (pwrite(FD[cpu], &data, sizeof(data), reg) != sizeof(data))
        {
            printf("Error: cpu %d reg %x",cpu, reg);
        }
}

