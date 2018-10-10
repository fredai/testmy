//Inspur (Beijing) Electronic Information Industry Co., Ltd.

//Written by LiuYu

/*
    File t_MicroStructure.cpp
    Version: V2.0.2
    Update: 2014-01-08
*/

//Teye modules for catching CPU and memory performance counter events.


//#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
//#include <iomanip>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
//#include "../include/teye.h"
#include "teye.h"
#include "t_MicroStructure.h"

//using namespace std;

extern struct teye_st s_teye;
extern PCM* instance;
static UncoreEventPCM *th_pcie;
static unsigned long long s_milliseconds = 0;

int std_swap(int *obj1, int *obj2)
{
    assert(obj1);
    assert(obj2);

    int p = 0; 

    p = *obj1;
    *obj1 = *obj2;
    *obj2 = p;

    return 0;
}


int tMicroStructure_MicroStructureInit(tMicroStructure *th) {
  
  printf("Teye Micro Structure  Monitor Initializing...\n");

  PCM* PcmInst = PCM_getInstance();
  int i = 0;

  th->cevent1 = (CoreEventPCM *)malloc(sizeof(CoreEventPCM)*PcmInst->getNumCores(PcmInst));
  th->cevent2 = (CoreEventPCM *)malloc(sizeof(CoreEventPCM)*PcmInst->getNumCores(PcmInst));
  th->fevent1 = (CoreFixedEventPCM *)malloc(sizeof(CoreFixedEventPCM)*PcmInst->getNumCores(PcmInst));
  th->fevent2 = (CoreFixedEventPCM *)malloc(sizeof(CoreFixedEventPCM)*PcmInst->getNumCores(PcmInst));
  th->uevent1 = (UncoreEventPCM *)malloc(sizeof(UncoreEventPCM)*PcmInst->getNumCores(PcmInst));
  th->uevent2 = (UncoreEventPCM *)malloc(sizeof(UncoreEventPCM)*PcmInst->getNumCores(PcmInst));

  for (i = 0; i < PcmInst->getNumCores(PcmInst); i++) {
    CoreEventPCM_init(&th->cevent1[i]);
    th->cevent1->CoreEventPCM(&th->cevent1[i]);

    CoreEventPCM_init(&th->cevent2[i]);
    th->cevent2->CoreEventPCM(&th->cevent2[i]);

    CoreFixedEventPCM_init(&th->fevent1[i]);
    th->fevent1->CoreFixedEventPCM(&th->fevent1[i]);

    CoreFixedEventPCM_init(&th->fevent2[i]);
    th->fevent2->CoreFixedEventPCM(&th->fevent2[i]);

    UncoreEventPCM_init(&th->uevent1[i]);
    th->uevent1->UncoreEventPCM(&th->uevent1[i]);

    UncoreEventPCM_init(&th->uevent2[i]);
    th->uevent2->UncoreEventPCM(&th->uevent2[i]);
  }
  //reset the PMUs
  PcmInst->resetPMU(PcmInst);

  return 0;
}

int tMicroStructure_MicroStructureStart(tMicroStructure *th) {

    pthread_t tid;
    pthread_attr_t attr;
    int err = -1;

  PCM* PcmInst = PCM_getInstance();

  s_milliseconds = get_current_millisecond();
  
  int i = 0;
  for (i = 0; i < PcmInst->getNumCores(PcmInst); i++) {
    //set core event select register
    th->cevent1[i].setCoreEventSelectRegister(&th->cevent1[i]);
    //read event counters
    PcmInst->getCounterValue(PcmInst, PcmInst->MsrDevice[i]);
    PcmInst->getFixedCounterValue(PcmInst, i);
    //set the cevent1 and fevent1 object member value
    th->cevent1[i].getCoreEvent(&th->cevent1[i]);
    th->fevent1[i].getCoreFixedEvent(&th->fevent1[i]);
  }

  for (i = 0; i < PcmInst->getNumSockets(PcmInst); i++) {
    //set uncore PCI PMON register
    PcmInst->setUncoreEventRegister(PcmInst, i);
    //read uncore event counters
    PcmInst->getUncoreCounterValue(PcmInst, i);
    th->uevent1[i].getUncoreEvent(&th->uevent1[i]);
  }

   err = sem_init(&IvySandy_pcie_sem_get, 0, 0);
    if (err != 0) {
        printf("sem_init error: %s\n", strerror(errno));
        return -1;
    }
    err = sem_init(&IvySandy_pcie_sem_put, 0, 1);
    if (err != 0) {
        printf("sem_init error: %s\n", strerror(errno));
        return -1;
    }
    
    err = pthread_attr_init(&attr);
    if (err != 0) {
        printf("pthread_attr_init error: %s\n", strerror(errno));
        return -1;
    }
    err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (0 == err) {
        th_pcie = &(th->uevent2[0]);
        err = pthread_create(&tid, &attr, th->uevent2[0].getPCIeBWEvent, &(th->uevent2[0]));
    }
    pthread_attr_destroy(&attr);

  return 0;
}

int tMicroStructure_MicroStructureRead(tMicroStructure *th) {

  float fx87 = 0.0;
  //For double precision
  float fssep = 0.0;
  float fssed = 0.0;
  float avx = 0.0;
  //For single precision
  float fssep_single= 0.0;
  float fssed_single = 0.0;
  float avx_single = 0.0;
  float cpi = 0.0;
  float mbw_r = 0.0;
  float mbw_w = 0.0;
  float pcie_r = 0.0;
  float pcie_w = 0.0;
  unsigned long long tmp_milliseconds = 0;
  unsigned long long cycle_milliseconds = 0;
  double cycle_seconds = 0.0;

  PCM* PcmInst = PCM_getInstance();
  int i = 0;

    tmp_milliseconds = get_current_millisecond();
  
  //th->uevent2[0].getPCIeBWEvent(&(th->uevent2[0]));
  
  for (i = 0; i < PcmInst->getNumCores(PcmInst); i++) {
    PcmInst->getCounterValue(PcmInst, PcmInst->MsrDevice[i]);
    PcmInst->getFixedCounterValue(PcmInst, i);
    th->cevent2[i].getCoreEvent(&th->cevent2[i]);
    th->fevent2[i].getCoreFixedEvent(&th->fevent2[i]);
  }

  for (i = 0; i < PcmInst->getNumSockets(PcmInst); i++) {
    PcmInst->getUncoreCounterValue(PcmInst, i);
    th->uevent2[i].getUncoreEvent(&th->uevent2[i]);
  }

  for (i = 0; i < PcmInst->getNumCores(PcmInst); i++) {
    fx87 += th->cevent2[i].getX87DoublePrecisionGFlops(&th->cevent2[i], th->cevent1[i]);
    fssep += th->cevent2[i].getSSEPackedDoublePrecisionGFlops(&th->cevent2[i], th->cevent1[i]);
    fssed += th->cevent2[i].getSSEScalarDoublePrecisionGFlops(&th->cevent2[i], th->cevent1[i]);
    avx += th->cevent2[i].getAVXPackedDoublePrecisionGFlops(&th->cevent2[i], th->cevent1[i]);
    fssep_single += th->cevent2[i].getSSEPackedSinglePrecisionGFlops(&th->cevent2[i], th->cevent1[i]);
    fssed_single += th->cevent2[i].getSSEScalarSinglePrecisionGFlops(&th->cevent2[i], th->cevent1[i]);
    avx_single += th->cevent2[i].getAVXPackedSinglePrecisionGFlops(&th->cevent2[i], th->cevent1[i]);
    cpi += th->fevent2[i].getCPI(&th->fevent2[i], th->fevent1[i]);
  }

  for (i = 0; i < PcmInst->getNumSockets(PcmInst); i++) {
    mbw_r += th->uevent2[i].getBandWidthRead(&th->uevent2[i], th->uevent1[i]);
    mbw_w += th->uevent2[i].getBandWidthWrite(&th->uevent2[i], th->uevent1[i]);
  }

  cycle_milliseconds = (tmp_milliseconds >= s_milliseconds) ? (tmp_milliseconds - s_milliseconds) : tmp_milliseconds;
  s_milliseconds = tmp_milliseconds;
  if (cycle_milliseconds == 0)
  {
        printf("cycle_milliseconds == 0, %llu\n", cycle_milliseconds);
        cycle_seconds = 0.0001;
  } else {
        cycle_seconds = cycle_milliseconds / 1000.0;
  }

  //Total Double GFlops
  s_teye.microstructure_values[0] = (fx87 + fssep + fssed + avx) / cycle_seconds;
  //Total Single GFlops
  s_teye.microstructure_values[1] = (fx87 + fssep_single + fssed_single + avx_single) / cycle_seconds;
  //X87 Double GFlops
  s_teye.microstructure_values[2] = fx87 / cycle_seconds;
  //SSE Packed Double GFlops
  s_teye.microstructure_values[3] = fssep / cycle_seconds;
  //SSE Scalar Double GFlops
  s_teye.microstructure_values[4] = fssed / cycle_seconds;
  //AVX Packed Double GFlops
  s_teye.microstructure_values[5] = avx / cycle_seconds;
  //SSE Packed Single GFlops
  s_teye.microstructure_values[6] = fssep_single / cycle_seconds;
  //SSE Scalar Single GFlops
  s_teye.microstructure_values[7] = fssed_single / cycle_seconds;
  //AVX Packed Single GFlops
  s_teye.microstructure_values[8] = avx_single / cycle_seconds;
  //SSE Packed Double VEC
  s_teye.microstructure_values[9] = fssep / (fx87 + fssep + fssed + avx);
  //AVX Packed Double VEC
  s_teye.microstructure_values[10] = avx / (fx87 + fssep + fssed + avx);
  //SSE Packed Single VEC
  s_teye.microstructure_values[11] = fssep_single
          / (fx87 + fssep_single + fssed_single + avx_single);
  //AVX Packed Single VEC
  s_teye.microstructure_values[12] = avx_single
          / (fx87 + fssep_single + fssed_single + avx_single);
  //CPI
  s_teye.microstructure_values[13] = cpi / PcmInst->getNumCores(PcmInst);
  //Total memory bandwidth
  s_teye.microstructure_values[14] = (mbw_r + mbw_w)  / cycle_seconds;
  //Memory Bandwidth Read
  s_teye.microstructure_values[15] = mbw_r / cycle_seconds;
  //Memory Bandwidth Write
  s_teye.microstructure_values[16] = mbw_w / cycle_seconds;

   sem_wait(&IvySandy_pcie_sem_get); 
  pcie_r = th->uevent2[0].getPCIeBandWidthRead(th_pcie);
  pcie_w = th->uevent2[0].getPCIeBandWidthWrite(th_pcie);
  sem_post(&IvySandy_pcie_sem_put); 
  
   s_teye.microstructure_values[17] = pcie_r;
   s_teye.microstructure_values[18] = pcie_w;

  std_swap((int *)&(th->cevent1), (int *)&(th->cevent2));
  std_swap((int *)&(th->fevent1), (int *)&(th->fevent2));
  std_swap((int *)&(th->uevent1), (int *)&(th->uevent2));

  return 0;
}

void tMicroStructure_UtMicroStructure(tMicroStructure *th) {
  free(th->cevent1);
  th->cevent1 = 0;

  free(th->cevent2);
  th->cevent2 = 0;

  free(th->fevent1);
  th->fevent1 = 0;

  free(th->fevent2);
  th->fevent2 = 0;

  free(th->uevent1);
  th->uevent1 = 0;

  free(th->uevent2);
  th->uevent2 = 0;
}
