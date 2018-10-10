//Inspur (Beijing) Electronic Information Industry Co., Ltd.

//Written by LiuYu

//The head file of t_MicroStructure.cpp

#ifndef _T_MICROSTRUCTURE_H_
#define _T_MICROSTRUCTURE_H_

#include "pcm.h"

typedef struct classtMicroStructure tMicroStructure;

struct classtMicroStructure {
    CoreEventPCM * cevent1;
    CoreEventPCM * cevent2;
    CoreFixedEventPCM * fevent1;
    CoreFixedEventPCM * fevent2;
    UncoreEventPCM * uevent1;
    UncoreEventPCM * uevent2;

    void (*tMicroStructure)(struct classtMicroStructure *th);
    void (*UtMicroStructure)(struct classtMicroStructure *th);

    int (*MicroStructureInit)(struct classtMicroStructure *th);
    int (*MicroStructureStart)(struct classtMicroStructure *th);
    int (*MicroStructureRead)(struct classtMicroStructure *th);
    //int MicroStructureStop();
    
};

int tMicroStructure_MicroStructureInit(tMicroStructure *th);
int tMicroStructure_MicroStructureStart(tMicroStructure *th);
int tMicroStructure_MicroStructureRead(tMicroStructure *th);
void tMicroStructure_UtMicroStructure(tMicroStructure *th) ;

#endif
