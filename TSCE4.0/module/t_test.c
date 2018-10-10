/*************************************************************************
	> File Name: test.c
	> Author: inspur
	> Mail: name@inspur.com 
	> Created Time: Tue 20 Oct 2015 04:09:21 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>

#include "con_define.h"
#include "framework.h"
#include "common.h"

#define TEST_MODULE_COL_NUM 4


static struct mod_info test_mod_info [] = {
	{"jack1", "\0"},
	{"jack2", "\0"},
	{"jack3", "\0"},
};


void 
test_start()
{

}


void 
test_read(struct module *mod)
{
	assert(mod != NULL);

	float jack1 = 1;
	float jack2 = 2;
	float jack3 = 3;

	
	assert(mod -> col == TEST_MODULE_COL_NUM);

	snprintf((mod->info[0]).index_data, LEN_32, "%.2f", jack1);
	snprintf((mod->info[1]).index_data, LEN_32, "%.2f", jack2);
	snprintf((mod->info[2]).index_data, LEN_32, "%.2f", jack3);


}

int
mod_register(struct module* mod)
{
    assert(mod != NULL);

    
	register_module_fields(mod, test_mod_info, \
						  TEST_MODULE_COL_NUM, test_start, test_read);
	return 0;
}
