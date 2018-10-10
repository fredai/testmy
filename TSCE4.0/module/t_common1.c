
/*
 * Copyright (C) Inspur(Bejing)
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
//#include"lua.h"
//#include"lualib.h"
//#include"lauxlib.h"

#include "con_define.h"
#include "framework.h"
#include "common.h"
#include "t_jack.h"

static struct mod_info cpu_mod_info[3];
#define CPU_MODULE_COL_NUM 10 

void 
cpu_start()
{
	FILE *fp;
	fp = fopen("common1", "wr");
	snprintf(cpu_mod_info[0].index_hdr,LEN_32, "%s", "jack1");
	snprintf(cpu_mod_info[1].index_hdr,LEN_32, "%s", "jack2");
	snprintf(cpu_mod_info[2].index_hdr,LEN_32, "%s", "jack3");
}

void 
cpu_read(struct module *mod)
{
	assert(mod != NULL);
/*
	lua_State *L = luaL_newstate();
	luaL_openlibs (L);
	luaL_dofile(L, "fun.lua");
	lua_getglobal(L, "add");
	lua_pushnumber(L, 2);
	lua_pushnumber(L, 3);
	lua_pushnumber(L, 4);
	lua_pcall(L, 3, 2, 0);
	double sum=0,ave = 0;
	if (lua_isnumber(L, 1))
	{
		sum = lua_tonumber(L, 1);
	}
	if (lua_isnumber(L, 2))
	{
		ave = lua_tonumber(L, 2);
	}
	lua_pop(L, 2);
	printf ("%.2f, %.2f\n", sum, ave);
	lua_close(L);
  */  


	
	snprintf((mod->info[0]).index_data, LEN_32, "%.2f", 1*1.0);
	snprintf((mod->info[1]).index_data, LEN_32, "%.2f", 10*19.1);
	snprintf((mod->info[2]).index_data, LEN_32, "%.2f", 100*10.0);

}


int
mod_register(struct module* mod)
{
    assert(mod != NULL);



    if(-1 == access("/proc/stat", R_OK)) {
            perror("access /proc/stat error:");
            return MODULE_FLAG_NOT_USEABLE;
        } 
	/* TODO: add decide module is usealbe in current HW and SW environment */
	register_module_fields(mod, cpu_mod_info, \
						  CPU_MODULE_COL_NUM, cpu_start, cpu_read);
	return 0;
}
