/*Inspur (Beijing) Electronic Information Industry Co., Ltd.
  
  Written by LiuYu
  
  File: pcm.h
  Version: V3.0.0
  Update: 2014-12-19
  
  Head file of pcm.cpp which is used for controling and 
  monitoring event select registers and performance counters.
*/

#ifndef _PCM_H_
#define _PCM_H_

#include "types.h"
#include "devices.h"

//class UncorePciMC;
//class UncoreMsrCBo;
typedef struct class_UncorePciMC UncorePciMC;
typedef struct class_UncoreMsrCBo UncoreMsrCBo;
typedef struct EventAndUmask EventAndUmask; 
/*
  CPU performance counter monitor
  This object can only be initialized once.
 */
 typedef struct class_PCM PCM;


struct class_PCM {

  void (*PCM)(struct class_PCM *th);

  uint32 NumCores;
  uint32 NumCoreGeneralCounters;
  uint32 NumCoreFixedCounters;
  uint32 NumSockets;
  uint32 CoreGeneralCounterWidth;
  uint32 CoreFixedCounterWidth;
  uint32 UncoreMCCounterWidth;
  uint32 UncoreCBoCounterWidth;
  uint32 PerformanceMonitorVersion;
  uint32 ThreadsPerCore;
  uint32 CPUFamily;
  uint32 CPUModel;
  //static PCM* instance;

  msr** MsrDevice;
  //Object 'UncorePciMCPMON' is used to control the uncore PCI PMON iMC events
  UncorePciMC** UncorePciMCPMON;
  //Object 'UncoreMsrCBoPMON' is used to control the uncore MSR PMON CBo events
  UncoreMsrCBo** UncoreMsrCBoPMON;
  //Array 'CoreCounterValue' contains the core general counter values
  uint64* CoreCounterValue;
  //Array 'CoreFixedCounterValue' contains the core fixed counter values
  uint64* CoreFixedCounterValue;
  //Array 'UncorePciCounterValue' contains the uncore pci counter values
  uint64* UncorePciCounterValue;
  //Array 'UncoreMsrCounterValue' contains the uncore msr counter values
  uint64* UncoreMsrCounterValue;

 //private:
  //extract core general counter value from counters
  uint64 (*extractCoreGeneralCounterValue)(struct class_PCM *th, uint64 bitvalue);
  //extract core fixed counter value from counters
  uint64 (*extractCoreFixedCounterValue)(struct class_PCM *th, uint64 bitvalue);
  //extract uncore general counter value from counters
  uint64 (*extractUncoreGeneralCounterValue)(struct class_PCM *th, uint64 bitvalue);
  //extrace uncore CBo counter value form CBo counters
  uint64 (*extractUncoreCBoCounterValue)(struct class_PCM *th, uint64 bitvalue);
  //extract value from a binary number bitvalue from bit begin
  //to bit end
  uint64 (*extractBits)(struct class_PCM *th, uint64 bitvalue, uint32 begin, uint32 end);
  //set the bit value to 1 from bit begin to bit end
  uint64 (*setBit)(struct class_PCM *th, uint32 begin, uint32 end);

 //public:
  //Function 'setEventRegister()' is used for setting the 
  //event select registers and performance counters and then
  //start counting.
  void (*setEventRegister)(struct class_PCM *th, EventAndUmask* parameters) ;
  //Function 'setUncoreEventRegister()' is used for setting the 
  //uncore Pci iMC event PMON registers and then start counting.
  void (*setUncoreEventRegister)(struct class_PCM *th, uint32 SocketId) ;
  //Function 'setUncoreCBoEventRegister' is used for setting the 
  //uncore Msr CBo event PMON registers and then start counting.
  void (*setUncoreCBoEventRegister)(struct class_PCM *th, uint32 SocketId, uint64 Opcode, 
				 uint64 TidField) ;

  //Function 'resetPMU()' is used to force the PMU reset.
  void (*resetPMU)(struct class_PCM *th);

  //Function 'getInstance()' will return the only PCM instance.
  //static PCM* (*getInstance)(struct class_PCM *th);


  //Function 'getCoreCounterValue()' will return the core 
  //performance counter values.
  void (*getCounterValue)(struct class_PCM *th, msr* CpuMsr);
  //Function 'getFixedCounterValue()' will return the core 
  //fixed performance counter values.
  void (*getFixedCounterValue)(struct class_PCM *th, uint32 CoreId);
  //Function 'getUncoreCounterValue()' will return the uncore 
  //Pci iMC PMON register counter values.
  void (*getUncoreCounterValue)(struct class_PCM *th, uint32 SocketId);
  //Function 'getUncoreCBoCounterValue()' will return the uncore 
  //Msr CBo PMON register counter values.
  void (*getUncoreCBoCounterValue)(struct class_PCM *th, uint32 SocketId);

  //Function 'getNumCores', 'getNumCoreGeneralCounters', 
  //'getNumCoreFixedCounters' and 'getNumSockets' will
  //return the maximun number of cores, maximun number of 
  //core general counters, maximun number of core fixed 
  //counters and maximun number of sockets.
  //Function 'getThreadsPerCore' and 'getCPUModel' will 
  //return the number of threads per CPU core and CPU model.
  uint32 (*getNumCores)(struct class_PCM *th) ;
  uint32 (*getNumCoreGeneralCounters)(struct class_PCM *th) ;
  uint32 (*getNumCoreFixedCounters)(struct class_PCM *th) ;
  uint32 (*getNumSockets)(struct class_PCM *th) ;
  uint32 (*getThreadsPerCore)(struct class_PCM *th) ;
  uint32 (*getCPUModel)(struct class_PCM *th) ;

  void (*UPCM)(struct class_PCM *th);
};

//Manipulate IMC uncore counters through PCI.
//Processoers like: SandyBridge-EP are using this way.
struct class_UncorePciMC {

 //private:
  //'PciMCDevices' are PCI mem file handles relate to their
  //sockets.
  PciMC** PciMCDevices;
  uint32 NumImcChannels;

  //forbidden
  //void (*UncorePciMC)(struct class_UncorePciMC *th);

 //public:
  //construct uncore memory controler event for each socket(SocketId)
  void (*UncorePciMC)(struct class_UncorePciMC *th, uint32 SocketId, uint32 MaxSockets, 
	      uint32 ProcessorModel, uint32 MaxCores);

  //Function 'setCounterValue()' is used for setting the uncore 
  //Pci PMON register values.
  void (*setCounterValue)(struct class_UncorePciMC *th);

  //Function 'getMCCounterRead()' and 'getMCCounterWrite()' will return 
  //the direct counter value read from the Pci PMON counter registers 
  //setted by function 'setCounterValue()'.
  uint64 (*getMCCounterRead)(struct class_UncorePciMC *th);
  uint64 (*getMCCounterWrite)(struct class_UncorePciMC *th);

  void (*UUncorePciMC)(struct class_UncorePciMC *th);
};

//Manipulate CBo uncore counters through MSR.
//Processoers like: SandyBridge-EP and IvyBridge-EP are using this way.
struct class_UncoreMsrCBo {

 //private:
  //'NumCBoxes' is CBox number related to a CPU socket.
  //'RelatedMsrOfSocket' is used to define a MSR device related to that socket.
  uint32 NumCBoxes;
  uint32 RelatedMsrOfSocket;

  //forbidden
  //void (*UncoreMsrCBo)(struct class_UncoreMsrCBo *th);
  //Function 'setCBoFilterRegister' is used to set the Opcode field for the 
  //CX_MSR_PMON_BOX_FILTER registers.
  void (*setCBoFilterRegister)(struct class_UncoreMsrCBo *th, uint32 ICBo, msr* MsrDevices, 
		       uint64 Opcode, uint32 ProcessorModel);

  //The 5 functions list bellow are used to calculate the correct 
  //register addresses.
  uint32 (*CX_MSR_PMON_CTRY)(struct class_UncoreMsrCBo *th, uint32 ICBo, uint32 CounterID) ;
  uint32 (*CX_MSR_PMON_BOX_FILTER)(struct class_UncoreMsrCBo *th, uint32 ICBo) ;
  uint32 (*CX_MSR_PMON_BOX_FILTER1)(struct class_UncoreMsrCBo *th, uint32 ICBo) ;
  uint32 (*CX_MSR_PMON_CTLY)(struct class_UncoreMsrCBo *th, uint32 ICBo, uint32 CounterID) ;
  uint32 (*CX_MSR_PMON_BOX_CTL)(struct class_UncoreMsrCBo *th, uint32 ICBo) ;

 //public:
  //construct uncore CBo event (PCI-Express) for each socket(SocketId)
  void (*UncoreMsrCBo)(struct class_UncoreMsrCBo *th, uint32 SocketId, uint32 NumCores, \
	       uint32 NumSockets, uint32 ThreadsPerCore);

  //Function 'setCounterValue()' is used for setting the uncore 
  //Msr PMON register values (here for PCI-Express read and write).
  void (*setCounterValue)(struct class_UncoreMsrCBo *th, uint64 Opcode, uint64 TidField);

  //Function 'getCBoCounterValue()' will return 
  //the direct counter values read from the Msr PMON counter registers 
  //setted by function 'setCounterValue()'.
  uint64 (*getCBoCounterValue)(struct class_UncoreMsrCBo *th);

  void (*UUncoreMsrCBo)(struct class_UncoreMsrCBo *th);
};

typedef struct class_CoreEventPCM CoreEventPCM ;
struct class_CoreEventPCM {

 //private:  
  uint64 X87DoublePrecision;
  uint64 SSEPackedDoublePrecision;
  uint64 SSEScalarDoublePrecision;
  uint64 AVXPackedDoublePrecision;
  uint64 SSEPackedSinglePrecision;
  uint64 SSEScalarSinglePrecision;
  uint64 AVXPackedSinglePrecision;

 //public:
  /*
    Function 'getCounterValue' in class PCM must be called first
    before calling any function list below.
    Functiion 'setCoreEventSelectRegister' will set the event 
    select register.
    Function 'getCoreEvent' will set the event member list above.
   */
  void (*setCoreEventSelectRegister)(struct class_CoreEventPCM *th);
  void (*getCoreEvent)(struct class_CoreEventPCM *th);

  float (*getX87DoublePrecisionGFlops)(struct class_CoreEventPCM *th, const CoreEventPCM old) ;
  float (*getSSEPackedDoublePrecisionGFlops)(struct class_CoreEventPCM *th, const CoreEventPCM old) ;
  float (*getSSEScalarDoublePrecisionGFlops)(struct class_CoreEventPCM *th, const CoreEventPCM old) ;
  float (*getAVXPackedDoublePrecisionGFlops)(struct class_CoreEventPCM *th, const CoreEventPCM old) ;
  float (*getSSEPackedSinglePrecisionGFlops)(struct class_CoreEventPCM *th, const CoreEventPCM old) ;
  float (*getSSEScalarSinglePrecisionGFlops)(struct class_CoreEventPCM *th, const CoreEventPCM old) ;
  float (*getAVXPackedSinglePrecisionGFlops)(struct class_CoreEventPCM *th, const CoreEventPCM old) ;
  

  void (*CoreEventPCM)(struct class_CoreEventPCM *th);
  /*
    X87DoublePrecision(0),
    SSEPackedDoublePrecision(0),
    SSEScalarDoublePrecision(0),
    AVXPackedDoublePrecision(0),
    SSEPackedSinglePrecision(0), 
    SSEScalarSinglePrecision(0), 
    AVXPackedSinglePrecision(0) {}
   */
  void (*UCoreEventPCM)();
};

typedef struct class_CoreFixedEventPCM  CoreFixedEventPCM ;
struct class_CoreFixedEventPCM {

 //private:  
  uint64 InstructionRetiredAny;
  uint64 CpuClockUnhaltedThread;
  uint64 CpuClockUnhaltedRef;

 //public:
  /*
    Function 'getCounterValue' in class PCM must be called first
    before calling any function list below.
    Function 'getCoreFixedEvent' will set the event member list above.
   */
  void (*getCoreFixedEvent)(struct class_CoreFixedEventPCM *th);

  uint64 (*getInstructionRetiredAny)(struct class_CoreFixedEventPCM *th, const CoreFixedEventPCM old) ;
  uint64 (*getCpuClockUnhaltedThread)(struct class_CoreFixedEventPCM *th, const CoreFixedEventPCM old) ;
  uint64 (*getCpuClockUnhaltedRef)(struct class_CoreFixedEventPCM *th, const CoreFixedEventPCM old) ;
  float (*getCPI)(struct class_CoreFixedEventPCM *th, const CoreFixedEventPCM old) ;
  

  void (*CoreFixedEventPCM)(struct class_CoreFixedEventPCM *th);
  /*
    InstructionRetiredAny(0),
    CpuClockUnhaltedThread(0),
    CpuClockUnhaltedRef(0) {}
  */
  void (*UCoreFixedEventPCM)();
};

typedef struct class_UncoreEventPCM UncoreEventPCM;
struct class_UncoreEventPCM {

 //private:
  uint64 BandWidthRead;
  uint64 BandWidthWrite;
  uint64 PCIeBandWidthRead;
  uint64 PCIeBandWidthWrite;

 //public:
  //Pointer member 'PCIeBandWidth' is used for data transfer between 
  //two process which are realized by Shared Memory.
  //This pointer must point to a uint64 type array which contains 
  //2 members at least.
  uint64* PCIeBandWidth;
  /*
    Function 'getUncoreCounterValue()' 
    in class PCM must be called first
    before calling any function list below.
    Function 'getUncoreEvent' will set the event member list above.
   */
  void (*getUncoreEvent)(struct class_UncoreEventPCM *th);

  //Function 'getPCIeBWEvent()' will set the value of member 
  //'PCIeBandWidthRead' and 'PCIeBandWidthWrite'
  //Because of sampling this function should 
  //be called in another precess/thread.
  //void (*getPCIeBWEvent)(struct class_UncoreEventPCM *th);
  void *(*getPCIeBWEvent)(void *th);

 //private:
  //Function 'getUncoreCBoEvent' will get the CBo counter value sampled by 
  //Opcode and TidFild.
  /*uint64 (*getUncoreCBoEvent)(struct class_UncoreEventPCM *th, uint64 Opcode, uint32 TimeInterval, \
			   const uint64 TidField = 0);*/
 uint64 (*getUncoreCBoEvent)(struct class_UncoreEventPCM *th, uint64 Opcode, uint32 TimeInterval, \
                uint64 TidField);


 //public:
  
  float (*getBandWidthRead)(struct class_UncoreEventPCM *th, const UncoreEventPCM old) ;
  float (*getBandWidthWrite)(struct class_UncoreEventPCM *th, const UncoreEventPCM old) ;
  float (*getPCIeBandWidthRead)(struct class_UncoreEventPCM *th) ;
  float (*getPCIeBandWidthWrite)(struct class_UncoreEventPCM *th) ;
  

  void (*UncoreEventPCM)(struct class_UncoreEventPCM *th);
/*
  : 
    BandWidthRead(0),
    BandWidthWrite(0),
    PCIeBandWidthRead(0),
    PCIeBandWidthWrite(0) {}
  */
  void (*UUncoreEventPCM)(struct class_UncoreEventPCM *th);

};

void PCM_PCM(struct class_PCM *th);
void PCM_UPCM(PCM *th);
PCM* PCM_getInstance() ;
void PCM_resetPMU(PCM *th);
void PCM_setEventRegister(PCM *th, EventAndUmask* parameters);
void PCM_setUncoreEventRegister(PCM *th, uint32 SocketId);
void PCM_setUncoreCBoEventRegister(PCM *th, uint32 SocketId, uint64 Opcode, uint64 TidField);
void PCM_getCounterValue(PCM *th, msr* CpuMsr);
void PCM_getFixedCounterValue(PCM *th, uint32 CoreId);
void PCM_getUncoreCounterValue(PCM *th, uint32 SocketId) ;
void PCM_getUncoreCBoCounterValue(PCM *th, uint32 SocketId);
uint64 PCM_extractCoreGeneralCounterValue(PCM *th, uint64 bitvalue);
uint64 PCM_extractCoreFixedCounterValue(PCM *th, uint64 bitvalue) ;
uint64 PCM_extractUncoreGeneralCounterValue(PCM *th, uint64 bitvalue);
uint64 PCM_extractUncoreCBoCounterValue(PCM *th, uint64 bitvalue);
uint64 PCM_extractBits(PCM *th, uint64 bitvalue, uint32 begin, uint32 end);
uint64 PCM_setBit(PCM *th, uint32 begin, uint32 end);
uint32 PCM_getNumCores(PCM *th);
uint32 PCM_getNumCoreGeneralCounters(PCM *th);
uint32 PCM_getNumCoreFixedCounters(PCM *th);
uint32 PCM_getNumSockets(PCM *th) ;
uint32 PCM_getThreadsPerCore(PCM *th);
uint32 PCM_getCPUModel(PCM *th);
void UncorePciMC_init(UncorePciMC *th);
void UncorePciMC_UncorePciMC(UncorePciMC *th, uint32 SocketId, uint32 MaxSockets, \
                                                                                                uint32 ProcessorModel, uint32 MaxCores) ;
void UncorePciMC_UUncorePciMC(UncorePciMC *th);
void UncorePciMC_setCounterValue(UncorePciMC *th);
uint64 UncorePciMC_getMCCounterRead(UncorePciMC *th) ;
uint64 UncorePciMC_getMCCounterWrite(UncorePciMC *th);
void UncoreMsrCBo_init(UncoreMsrCBo *th);
void UncoreMsrCBo_UncoreMsrCBo(UncoreMsrCBo *th, uint32 SocketId, uint32 NumCores, 
                   uint32 NumSockets, uint32 ThreadsPerCore) ;
void UncoreMsrCBo_UUncoreMsrCBo(UncoreMsrCBo *th); 
void UncoreMsrCBo_setCounterValue(UncoreMsrCBo *th, uint64 Opcode, uint64 TidField);
void UncoreMsrCBo_setCBoFilterRegister(UncoreMsrCBo *th, uint32 ICBo, 
                        msr* MsrDevices, 
                        uint64 Opcode, 
                        uint32 ProcessorModel);
uint64 UncoreMsrCBo_getCBoCounterValue(UncoreMsrCBo *th) ;
uint32 UncoreMsrCBo_CX_MSR_PMON_CTRY(UncoreMsrCBo *th, uint32 ICBo, uint32 CounterID);
uint32 UncoreMsrCBo_CX_MSR_PMON_BOX_FILTER(UncoreMsrCBo *th, uint32 ICBo) ;
uint32 UncoreMsrCBo_CX_MSR_PMON_BOX_FILTER1(UncoreMsrCBo *th, uint32 ICBo) ;
uint32 UncoreMsrCBo_CX_MSR_PMON_CTLY(UncoreMsrCBo *th, uint32 ICBo, uint32 CounterID) ;
uint32 UncoreMsrCBo_CX_MSR_PMON_BOX_CTL(UncoreMsrCBo *th, uint32 ICBo);
void CoreEventPCM_init(CoreEventPCM *th);
void CoreEventPCM_CoreEventPCM(struct class_CoreEventPCM *th);
void  CoreEventPCM_UCoreEventPCM() ;
void CoreEventPCM_setCoreEventSelectRegister(CoreEventPCM *th);
void CoreEventPCM_getCoreEvent(CoreEventPCM *th);
float CoreEventPCM_getX87DoublePrecisionGFlops(CoreEventPCM *th, const CoreEventPCM old) ;
float CoreEventPCM_getSSEPackedDoublePrecisionGFlops(CoreEventPCM *th, const CoreEventPCM old);
float CoreEventPCM_getSSEScalarDoublePrecisionGFlops(CoreEventPCM *th, const CoreEventPCM old);
float CoreEventPCM_getAVXPackedDoublePrecisionGFlops(CoreEventPCM *th, const CoreEventPCM old);
float CoreEventPCM_getSSEPackedSinglePrecisionGFlops(CoreEventPCM *th, const CoreEventPCM old) ;
float CoreEventPCM_getSSEScalarSinglePrecisionGFlops(CoreEventPCM *th, const CoreEventPCM old);
float CoreEventPCM_getAVXPackedSinglePrecisionGFlops(CoreEventPCM *th, const CoreEventPCM old) ;
void CoreFixedEventPCM_init(CoreFixedEventPCM *th);
void CoreFixedEventPCM_CoreFixedEventPCM(struct class_CoreFixedEventPCM *th);
void CoreFixedEventPCM_UCoreFixedEventPCM() ;
void CoreFixedEventPCM_getCoreFixedEvent(CoreFixedEventPCM *th);
uint64 CoreFixedEventPCM_getInstructionRetiredAny(CoreFixedEventPCM *th, const CoreFixedEventPCM old) ;
uint64 CoreFixedEventPCM_getCpuClockUnhaltedThread(CoreFixedEventPCM *th, const CoreFixedEventPCM old) ;
uint64 CoreFixedEventPCM_getCpuClockUnhaltedRef(CoreFixedEventPCM *th, const CoreFixedEventPCM old);
float CoreFixedEventPCM_getCPI(CoreFixedEventPCM *th, const CoreFixedEventPCM old);
void UncoreEventPCM_init(UncoreEventPCM *th);
void UncoreEventPCM_UncoreEventPCM(struct class_UncoreEventPCM *th);
void UncoreEventPCM_UUncoreEventPCM();
void UncoreEventPCM_getUncoreEvent(UncoreEventPCM *th) ;
//void UncoreEventPCM_getPCIeBWEvent(UncoreEventPCM *th) ;
void *UncoreEventPCM_getPCIeBWEvent(void *th) ;
uint64 UncoreEventPCM_getUncoreCBoEvent(UncoreEventPCM *th, uint64 Opcode,  
                         uint32 TimeInterval, 
                         const uint64 TidField);
float UncoreEventPCM_getBandWidthRead(UncoreEventPCM *th, const UncoreEventPCM old);
float UncoreEventPCM_getBandWidthWrite(UncoreEventPCM *th, const UncoreEventPCM old);
float UncoreEventPCM_getPCIeBandWidthRead(UncoreEventPCM *th);
float UncoreEventPCM_getPCIeBandWidthWrite(UncoreEventPCM *th); 

#endif
