
/*
 * Copyright (C) Inspur(Beijing)
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "getdata.h"
#include "clr_config.h"
#include "clr_define.h"
#include "error_handle.h"
#include "protocol.h"
#include "data.h"
#include "common_util.h"
#include "util.h"


/* max connect times */
#define MAX_CONNECT_TIMES 60


/* global module struct */
app_clr_module_t app_clr_module;


static int init_module_array();

int
analyse_index_info_cmd(int argc, char *argv[], config_mod_info_t *g_mod_info)
{
	int i;
    config_mod_info_t *head;
    head = g_mod_info;
	
	for (i=1; i<argc; i++)
	{
		config_mod_info_item_t *item;
		
		printf("debug module %s\n", argv[i]);
		config_mod_info_t *mod_info = (config_mod_info_t*) \
									  (malloc(sizeof(config_mod_info_t)));
		if (mod_info == NULL) {
			err_sys("malloc error");
			continue;
		}
        memset(mod_info, 0, sizeof(config_mod_info_t));
		item = &(mod_info->info);


		strncpy(item->mod_name, argv[i],strlen(argv[i]));
		head->next = mod_info;
	    head = head->next;

		item->index_cnt=1;
	}
	return 0;
}

int 
main(int argc, char *argv[]) 
{
    int ret;
    app_clr_config_t app_clr_config;
    config_mod_info_t   mod_info_head; 

    bzero(&app_clr_module, sizeof(app_clr_module));
    bzero(&mod_info_head, sizeof(config_mod_info_t));

    /* index info */
	 if(argc == 1) {
		ret = analyse_index_info_config(GETDATA_INDEX_CONFIG_PATH, &mod_info_head);
		if (ret < 0) {
			err_exit("init tsce index configure file error");
		}
		FLAG=0;
	 }
	 else{
		ret = analyse_index_info_cmd(argc, argv, &mod_info_head);
		FLAG=1;
	}
    /* log */
    cs_log_start(app_clr_config.log_level);

    /* record pid */
	FILE * pid_file = fopen(CLR_PID_FILE_PATH, "w");
    if (pid_file != NULL) {
        fprintf(pid_file, "%d", getpid());
        fclose(pid_file);
    }
  
	init_module_array(&mod_info_head);
    /* load modules */
    load_modules();

    const int INTERVAL = 1;


    /* package */
    app_pkg_t app_pkg;
    app_data_t *app_data = &app_pkg.body;
    begin_collect_data();
    
    while (1) {
        struct timeval start_time = timeval_current();
        get_app_data();

        memset(app_data, 0, sizeof(app_data_t));
        /* comple appfeature data, fill corresponding error mask */
        complete_app_data(app_data, &mod_info_head);
	printf("%s\n",app_data->buffer);
        /* send */

        smart_sleep(INTERVAL, &start_time);
    }


    free_modules();
    free_index_info_config(&mod_info_head);
    cs_log_end;
    exit(0);
}


static int
init_module_array(config_mod_info_t *mod_info_head)
{
    assert(mod_info_head != NULL);

    int num = 0;
    struct module *mod;
    config_mod_info_t *t;
    config_mod_info_item_t *item;

    t = mod_info_head -> next;
    while  (t != NULL) {
        item = &(t -> info);
        num = app_clr_module.total_mod_num;
        mod = &app_clr_module.mods[num];
        snprintf(mod -> name, sizeof(mod -> name), "%s", item -> mod_name);
        app_clr_module.total_mod_num++;
        t = t -> next;
    }

    return 0;
}
