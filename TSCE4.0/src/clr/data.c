
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "data.h"
#include "clr_config.h"
#include "framework.h"

static int collect_data_from_modules(void);

static int add_error_mask(char *data_buf, int error, int cnt);

static int compare_index_data(char *data_buf, struct module *mod, \
		config_mod_info_item_t *item);

static int complete_module_data(char *data_buf, config_mod_info_t *t);


static int
collect_data_from_modules(void)
{

	int i, num, j=0;
	struct module *mod;
	void (*c_read) (struct module *mod);

	num = app_clr_module.total_mod_num;
	for (i = 0; i < num; i++) {
		mod = &app_clr_module.mods[i];

		if (mod -> flag != MODULE_FLAG_NORMAL) {
			/* is script ? */
			if (mod -> flag == SCRIPT_FLAG_NORMAL) {
				char script_read[LEN_64]={0};
				snprintf(script_read, LEN_64, "%s read;echo $?", script_t.script_path[j]);
				script_t.fp[j] = popen(script_read, "r");
				fgets(script_t.buffer[j],sizeof(script_t.buffer[j]),script_t.fp[j]);
				if((strstr(script_t.buffer[j],":")) || (strstr(script_t.buffer[j],"=")))
				{
					int len = strlen(script_t.buffer[j]);
					if (script_t.buffer[j][len-1] == '\n') {
						script_t.buffer[j][len-1] =0;
					}
					pclose(script_t.fp[j]);
					j++;
				}else {
					return -1;
				}
			}
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
	int i, num, j=0;
	struct module *mod;
	void (*c_start) (void);

	num = app_clr_module.total_mod_num;
	for (i = 0; i < num; i++) {
		mod = &app_clr_module.mods[i];

		if (mod -> flag != MODULE_FLAG_NORMAL) {
			/* is script ? */
			if (mod -> flag == SCRIPT_FLAG_NORMAL) {
				char script_start[LEN_64]={0};
				snprintf(script_start, LEN_64, "%s start", script_t.script_path[j]);
				script_t.fp[j] = popen(script_start, "r");
				fgets(script_t.buffer[j],sizeof(script_t.buffer[j]),script_t.fp[j]);
				pclose(script_t.fp[j]);
				j++;
			}
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
	int ret = collect_data_from_modules();
    return ret;
}


static int
add_error_mask(char *data_buf, int error, int cnt)
{
	assert(data_buf != NULL && error < 0 && cnt > 0);
	
	int i, len;
	char temp[8];

	len = 0;
	for (i = 0; i < cnt; i++){
		memset(temp, 0, sizeof(temp));
		snprintf(temp, sizeof(temp), "%d", error);
		strncat(data_buf, temp, strlen(temp));
		strcat(data_buf, TEYE_DATA_DELIMTER);
		len += (strlen(temp) + strlen(TEYE_DATA_DELIMTER));
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

	for (i = 0; i < item -> index_cnt; i++) {
		for (j = 0; j < mod -> col; j++) {
			info = &(mod -> info[j]);
			real_index_name = info -> index_hdr;
			conf_index_name = (item -> index_info[i]).index_name;

			if (!strcmp(conf_index_name, real_index_name)) {
				strncat(data_buf, info -> index_data, strlen(info -> index_data));
				strcat(data_buf, TEYE_DATA_DELIMTER);
				break;
			}
		}
		/* there is no this index in the registered module */
		if (j == mod -> col) {
			char temp_buf[LEN_32] = {0};
			add_error_mask(temp_buf, MODULE_FLAG_INDEX_NOT_EXIST, 1);
			strncat(data_buf, temp_buf, strlen(temp_buf));
		}
	}
	return 0;
}




static int
deal_script_data(char *data_buf, config_mod_info_item_t *item)
{
	/* judge script key_value */
	char *arry[SCRIPT_KV_NUM], *saveptr_it, *saveptr_kv;
	char *delim_it=",";
	char *delim_kv=":=";
	int j=0;
	
	arry[j] = strtok_r(script_t.buffer[script_t.script_j], delim_it, &saveptr_it);
	script_t.kv[j].k = strtok_r(arry[j], delim_kv, &saveptr_kv);
	script_t.kv[j].v = strtok_r(NULL, delim_kv, &saveptr_kv);

	if (!strcmp(script_t.kv[j].k,  (item->index_info[j]).index_name)) {
		if ( script_t.kv[j].v == NULL) 
			strncat(data_buf, "-1", strlen("-1"));
		else 
			strncat(data_buf, script_t.kv[j].v, strlen(script_t.kv[j].v));
		strcat(data_buf, TEYE_DATA_DELIMTER);
	} else {
		add_error_mask(data_buf, MODULE_FLAG_INDEX_NOT_EXIST, 1);
	}

	while((arry[++j]=strtok_r(NULL, delim_it, &saveptr_it)) != NULL) {
		script_t.kv[j].k = strtok_r(arry[j], delim_kv, &saveptr_kv);
		script_t.kv[j].v = strtok_r(NULL, delim_kv, &saveptr_kv);
//		printf("__file__=%s, __line__=%d, k=%s, v=%s\n", __FILE__,__LINE__, script_t.kv[j].k, script_t.kv[j].v);
		if (!strcmp(script_t.kv[j].k,  (item->index_info[j]).index_name)) {
			if ( script_t.kv[j].v == NULL) 
				strncat(data_buf, "-1", strlen("-1"));
			else 
				strncat(data_buf, script_t.kv[j].v, strlen(script_t.kv[j].v));
			strcat(data_buf, TEYE_DATA_DELIMTER);
		} else {
			add_error_mask(data_buf, MODULE_FLAG_INDEX_NOT_EXIST, 1);
		}
	}


	script_t.script_j++;	//script number

	/* if script echo data format like this "echo10,14,12,54" use this drictly */
//	strncat(data_buf, script_t.buffer[script_t.script_j], strlen(script_t.buffer[script_t.script_j])-1);
//	strcat(data_buf, TEYE_DATA_DELIMTER);
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
				/* if is script */
				if(mod -> flag == SCRIPT_FLAG_NORMAL) {				
					deal_script_data(temp_buf, item);
				}else {
					add_error_mask(temp_buf, mod -> flag, item -> index_cnt);
				}
			} else {
				compare_index_data(temp_buf, mod, item);
			}
            break;
		}
	}
	
	/* it can't find the module info in the loaded modules */
	if (i == num) {
		add_error_mask(temp_buf, MODULE_FLAG_NOT_EXIST, item -> index_cnt);	
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

