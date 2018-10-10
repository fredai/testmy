
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

#define SCRIPT_FLAG_NORMAL 1
#define SCRIPT_NUM 50
#define SCRIPT_DATA 1024
#define SCRIPT_KV_NUM 32


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

struct k_v {
	char *k;
	char *v;
};

struct script {
	FILE *fp[SCRIPT_NUM];
	char script_path[SCRIPT_NUM][LEN_64];
	char buffer[SCRIPT_NUM][SCRIPT_DATA];
	struct k_v kv[SCRIPT_KV_NUM];
	int script_j;
};

extern struct script script_t;
int load_modules(void);

int free_modules(void);


#endif
