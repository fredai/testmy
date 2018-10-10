
/*
 * Copyright (C) Inspur(Beijing)
 */


#ifndef TEYE_FRAMEWORK_H
#define TEYE_FRAMEWORK_H


#include "con_define.h"


#define MODULE_REGISTER_FUNCTION "mod_register"


#define MODULE_FLAG_NORMAL 0
#define MODULE_FLAG_NOT_EXIST -1
#define MODULE_FLAG_ILLEGAL -2
#define MODULE_FLAG_INDEX_NOT_EXIST -3
#define MODULE_FLAG_NOT_USEABLE -4


struct mod_info {
	char index_hdr[LEN_32];
	char index_data[LEN_32];
};


struct module {
	char 	name[LEN_32];
	int		flag;
	struct 	mod_info *info;
	int		col;
	void	*lib;

	void (*c_start) (void);	
	void (*c_read)	(struct module*);
};


int load_modules(void);

int free_modules(void);


#endif
