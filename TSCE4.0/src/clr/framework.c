
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "framework.h"
#include "clr_config.h"
#include <dlfcn.h>
#include <string.h>
#include <assert.h>


int
load_modules(void)
{
	int i, num, j=0;
	struct module *mod;
	char mod_path[LEN_64] = {0};
	int (*mod_register) (struct module*);

	num = app_clr_module.total_mod_num;
	for (i = 0; i < num; i++) {
		mod = &app_clr_module.mods[i];
		assert(mod->lib == NULL);
		
		memset(mod_path, 0, sizeof(mod_path));
		snprintf(mod_path, LEN_64, "%s/lib%s.so", MODULE_SEARCH_PATH, mod->name);
		/* get the libaray */
		if ( (mod->lib = dlopen(mod_path, RTLD_NOW | RTLD_GLOBAL)) == NULL) {
//			mod->flag = MODULE_FLAG_NOT_EXIST;
			/* get the script */
			snprintf(mod_path, LEN_64, "%s/%s", SCRIPT_SEARCH_PATH, mod->name);
			if(access(mod_path, F_OK | X_OK) == 0) {
				strcpy(script_t.script_path[j], mod_path);
				mod->flag = SCRIPT_FLAG_NORMAL;
				j++;
			}
			else {
				mod->flag = MODULE_FLAG_NOT_EXIST;
			}
		} else {
			mod_register = dlsym(mod->lib, MODULE_REGISTER_FUNCTION);
			if (mod_register == NULL) {
				mod->flag = MODULE_FLAG_ILLEGAL;
			} else {
				mod->flag = mod_register(mod);
			}
		}
	};

	return 0;
}


int
free_modules(void)
{
	int i, num;
	struct module *mod;

	num = app_clr_module.total_mod_num;
	for (i = 0; i < num; i++) {
		mod = &app_clr_module.mods[i];
		if (mod->lib) {
			dlclose(mod->lib);
		}
	}

	return 0;
}
