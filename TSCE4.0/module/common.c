#include <stdio.h>
#include <assert.h>
#include <sys/time.h>


#include "con_define.h"
#include "framework.h"

void 
register_module_fields(struct module *mod, struct mod_info *info, \
						int col, void *c_start, void *c_read)
{
assert(mod != NULL && info != NULL && col != 0 
				   && c_start != NULL && c_read != NULL);

	mod -> info = info;
	mod -> col = col;
	mod -> c_start = c_start;
	mod -> c_read = c_read;
	return;
}

unsigned long long get_current_millisecond() 
{
	struct timeval tv = {0,0};

	if (-1 == gettimeofday(&tv, NULL)) {
		perror("gettimeofday");	
		exit(1);
	}
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

