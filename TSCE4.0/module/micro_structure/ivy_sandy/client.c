/*Inspur (Beijing) Electronic Information Industry Co., Ltd.
  
  Written by LiuYu
  
  File: client.cpp
  Version: Based on command line V2.0.2
  Update: 2014-01-08

  The main program of teyemon deamon.
*/


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "t_MicroStructure.h"
#include "teye.h"

#include "cpuinfo.h"
#include "cpuinfo.c"
#include "devices.h"
#include "devices.c"
#include "pcm.h"
#include "pcm.c"
#include "t_MicroStructure.c"

#define IvySandy_MODULE_COL_NUM 19
static struct mod_info IvySandy_mod_info[] = {
    {"Total_DP_Gflops", "\0"},
    {"Total_SP_Gflops", "\0"}, 
    {"X87_Gflops", "\0"}, 
    {"SSE_DP_Packed", "\0"},
    {"SSE_DP_Scalar", "\0"},  
    {"AVX_DP_Packed", "\0"}, 
    {"SSE_SP_Packed", "\0"}, 
    {"SSE_SP_Scalar", "\0"}, 
    {"AVX_SP_Packed", "\0"}, 
    {"SSE_DP_VEC", "\0"}, 
    {"AVX_DP_VEC", "\0"}, 
    {"SSE_SP_VEC", "\0"}, 
    {"AVX_SP_VEC", "\0"},
    {"cpi","\0"},
    {"memBW_total", "\0"},
    {"memBW_read","\0"},
    {"memBW_write","\0"},
    {"pcie_read", "\0"},
    {"pcie_write", "\0"}
};



//extern "C" int TeyeInit();
//extern "C" struct teye_st GetTeyeData();
//extern "C" void TeyeStop();
short int TeyeErrorCode = 0;

struct  teye_st s_teye;
tMicroStructure* tms;

/**
 * start函数，接收到SI_START后执行。
 * 依次执行各个模块的start函数。
 */ 
void c_start(tMicroStructure* tms)
{
  //printf("Starting...\n");
  tms->MicroStructureStart(tms);
}

/**
 * stop函数，接收到SI_STOP后执行。
 * 依次执行各个模块的stop函数。
 */ 
void c_stop(void)
{
  //printf("Stoping...\n");
  //ib_stop();
}

/**
 * read函数，接收到SI_READ后执行。
 * 依次执行各个模块的read函数。
 * 每个模块的read函数负责设置s_teye的各个字段。 
 */ 
void c_read(tMicroStructure* tms)
{
  //printf("Reading...\n");
  tms->MicroStructureRead(tms);

}

/**
 * 本程序初始化时执行一次。
 * 主要作用为收集机器基本信息。 
 */ 
void c_init(tMicroStructure* tms)
{
  FILE * p = 0;
  printf("teyemon initing...\n");
  //memset(&s_teye, 0, sizeof(s_teye));
  //memset(&teye, 0, sizeof(teye));             
  s_teye.cpu_num=sysconf(_SC_NPROCESSORS_ONLN);
  gethostname(s_teye.hostname, sizeof(s_teye.hostname));
  p = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
  if (p) {
    fscanf(p, "%d", &s_teye.cache_line_size);
    fclose(p);
  }
  
  if(0 == TeyeErrorCode) {
    if( -1 == tms->MicroStructureInit(tms))
      exit(1);
  }
}

int TeyeInit() {
  tms = (tMicroStructure *)malloc(sizeof(tMicroStructure));
  tms->MicroStructureInit = tMicroStructure_MicroStructureInit;
  tms->MicroStructureRead = tMicroStructure_MicroStructureRead;
  tms->MicroStructureStart = tMicroStructure_MicroStructureStart;
  tms->UtMicroStructure = tMicroStructure_UtMicroStructure;

  c_init(tms);
        
  if (0 == TeyeErrorCode) {
    c_start(tms);
  }

  return TeyeErrorCode;
}

struct teye_st GetTeyeData() {
  c_read(tms);

  return s_teye;
}

void IvySandy_read(struct module *mod)
{
    struct teye_st teye_data = GetTeyeData();

    snprintf ( (mod->info [0]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[0]);
    snprintf ( (mod->info [1]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[1]);
    snprintf ( (mod->info [2]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[2]);
    snprintf ( (mod->info [3]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[3]);
    snprintf ( (mod->info [4]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[4]);
    snprintf ( (mod->info [5]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[5]);
    snprintf ( (mod->info [6]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[6]);
    snprintf ( (mod->info [7]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[7]);
    snprintf ( (mod->info [8]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[8]);
    snprintf ( (mod->info [9]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[9]);
    snprintf ( (mod->info [10]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[10]);
    snprintf ( (mod->info [11]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[11]);
    snprintf ( (mod->info [12]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[12]);
    snprintf ( (mod->info [13]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[13]);
    snprintf ( (mod->info [14]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[14]);
    snprintf ( (mod->info [15]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[15]);
    snprintf ( (mod->info [16]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[16]);
    snprintf ( (mod->info [17]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[17]);
    snprintf ( (mod->info [18]).index_data, LEN_32, "%.2f", teye_data.microstructure_values[18]);
   
    return;
}

void TeyeStop() {
  c_stop();
}


