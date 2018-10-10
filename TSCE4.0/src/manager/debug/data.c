
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "data.h"
#include "clr_config.h"
#include "framework.h"
#include "getdata.h"

static int collect_data_from_modules(void);

static int add_error_mask(char *data_buf, config_mod_info_item_t *item, int cnt);

static int compare_index_data(char *data_buf, struct module *mod, \
		config_mod_info_item_t *item);

static int complete_module_data(char *data_buf, config_mod_info_t *t);

int lua_regist(void);
void lua_load_file(char *lua_file);
void lua_getval(char *lua_file, struct module *mod);

static int
collect_data_from_modules(void)
{

	int i, num;
	struct module *mod;
	void (*c_read) (struct module *mod);

	
	num = app_clr_module.total_mod_num;
	for (i = 0; i < num; i++) {
		mod = &app_clr_module.mods[i];

		if (mod -> flag != MODULE_FLAG_NORMAL) {
			continue;
		}
		c_read = mod -> c_read;
		assert (c_read != NULL);
		
		c_read(mod);
	}

	return 0;
}


int
begin_collect_data(void)
{
	int i, num;
	struct module *mod;
	void (*c_start) (void);

	num = app_clr_module.total_mod_num;
	for (i = 0; i < num; i++) {
		mod = &app_clr_module.mods[i];

		if (mod -> flag != MODULE_FLAG_NORMAL) {
			continue;
		}
	
		c_start = mod -> c_start;
		assert (c_start != NULL);

		c_start();
	}
	return 0;
}


int 
get_app_data(void)
{
	collect_data_from_modules();
    return 0;
}


static int
add_error_mask(char *data_buf, config_mod_info_item_t *item, int cnt)
{
	assert(data_buf != NULL && item!= NULL && cnt > 0);
	
	int i, len;
//	char temp[8];
	char *conf_index_name;

	len = 0;
	for (i = 0; i < cnt; i++){
		conf_index_name = (item -> index_info[i]).index_name;
		strncat(data_buf, conf_index_name, strlen(conf_index_name));
		strcat(data_buf, GETDATA_TAGS);
		strncat(data_buf, "-1", strlen("-1"));
		strcat(data_buf, TEYE_DATA_DELIMTER);
	}
	return len;
}


static int
compare_index_data(char *data_buf, struct module *mod, config_mod_info_item_t *item)
{
	assert(data_buf != NULL && mod != NULL && item != NULL);
	
	int i, j;
	struct mod_info	*info;
	char *real_index_name, *conf_index_name;

	if(FLAG==0){
		for (i = 0; i < item -> index_cnt; i++) {
			for (j = 0; j < mod -> col; j++) {
				info = &(mod -> info[j]);
				real_index_name = info -> index_hdr;
				conf_index_name = (item -> index_info[i]).index_name;

				if (!strcmp(conf_index_name, real_index_name)) {
					strncat(data_buf, info -> index_hdr, strlen(info -> index_hdr));
					strcat(data_buf, GETDATA_TAGS);
					strncat(data_buf, info -> index_data, strlen(info -> index_data));
					strcat(data_buf, TEYE_DATA_DELIMTER);
					break;
				}
			}
			/* there is no this index in the registered module */
			if (j == mod -> col) {
				char temp_buf[LEN_32] = {0};
				add_error_mask(temp_buf, item, 1);
				strncat(data_buf, temp_buf, strlen(temp_buf));
			}
		}
	}
	if(FLAG==1)	{
		for (i = 0; i < item -> index_cnt; i++) {
			for (j = 0; j < mod -> col; j++) {
				info = &(mod -> info[j]);
				real_index_name = info -> index_hdr;

				strncat(data_buf, info -> index_hdr, strlen(info -> index_hdr));
				strcat(data_buf, GETDATA_TAGS);
				strncat(data_buf, info -> index_data, strlen(info -> index_data));
				strcat(data_buf, TEYE_DATA_DELIMTER);
			}
		}
	}
	return 0;
}


static int
complete_module_data(char *data_buf, config_mod_info_t *t)
{
	assert(data_buf != NULL && t != NULL);

	int i, num;
	char temp_buf[LEN_10240] = {0};
	config_mod_info_item_t *item;
	struct module *mod;

	num = app_clr_module.total_mod_num;
	item = &(t -> info);
	for (i = 0; i < num; i++) {
		mod = &app_clr_module.mods[i];

		if(0 == strcmp(item->mod_name, mod->name)) {
			if(mod -> flag != MODULE_FLAG_NORMAL) {
				add_error_mask(temp_buf, item, item -> index_cnt);
			} else {
				compare_index_data(temp_buf, mod, item);
			}
            		break;
		}
	}
	
	/* it can't find the module info in the loaded modules */
	if (i == num) {
		add_error_mask(temp_buf, item, item -> index_cnt);	
	}
	strncat(data_buf, temp_buf, strlen(temp_buf));
	return 0;
}



int
complete_app_data(app_data_t *app_data, config_mod_info_t * mod_info_head)
{
	assert(app_data != NULL && mod_info_head != NULL);
	
	config_mod_info_t	*t;
	char		*buf;
    int         len;

	t = mod_info_head -> next;
	buf = app_data -> buffer;
	while (t != NULL) {
		complete_module_data(buf, t);
		t = t -> next;
	}
 
    len = strlen(buf);
    if (strcmp(buf + len - 1, TEYE_DATA_DELIMTER) == 0) {
        buf[len - 1] = '\0';
    }

	return 0;
}


int 
produce_app_pkg_head(app_hdr_t * app_head) 
{
    if ( app_head == NULL ) {
        return -1;
    }
    bzero ( app_head, sizeof ( app_hdr_t ) );
    app_head -> request_type = REQUEST_TYPE_POST;
    app_head -> data_type = REQUEST_POST_NORMAL_DATA;
    app_head -> len = sizeof ( app_data_t );
    return 0;
}

