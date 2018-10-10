#ifndef MODULE_COMMON_H
#define MODULE_COMMON_H


#include "framework.h"

typedef unsigned int uint32;
typedef unsigned long long uint64;

#define GB  (1024*1024*1024)
#define MB  (1024*1024)

//#define DEBUG   1

void 
register_module_fields(struct module *mod, struct mod_info *info, \
						int col, void *c_start, void *c_read);
						
unsigned long long  get_current_millisecond();						

#endif
