#ifndef MODULE_PCI_H
#define MODULE_PCI_H

/*
 * Copyright (C) Inspur(Bejing)
 * 
 */

extern void pci_init(uint32 testDevice, uint32 MAX_NUM_DEVICES, char *pci_Device[]);
extern void pci_finalize(uint32 MAX_NUM_DEVICES);
extern void pci_write32(int socketId, uint32 device_index, char *pci_Device[], uint32 reg, uint32 data);
extern uint64 pci_read64(int socketId, uint32 device_index, char *pci_Device[], uint32 reg);

#endif
