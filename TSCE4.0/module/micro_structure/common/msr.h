#ifndef MODULE_MSR_H
#define MODULE_MSR_H

/*
 * Copyright (C) Inspur(Bejing)
 * 
 */

extern void msrInit(void);
extern void msrFinalize(void);
extern uint64 msrRead( const int cpu, uint32 reg);
extern void msrWrite( const int cpu, uint32 reg, uint64 data);

#endif
