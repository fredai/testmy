
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "svr_config.h"
#include "error_handle.h"


#define TEMP_STOP_RESULT_FILE "./stop_result"
#define TEYE_CLIENT_STOP_SUCCESS "%s:success "
#define TEYE_CLIENT_STOP_FAILED  "%s:failed "

static int check_args(int argc, char *argv[]);


int 
main(int argc, char *argv[]) 
{
    int ret;
    FILE *f;

    ret = check_args(argc, argv);
    if (ret < 0) {
        err_exit("Usage: %s -d path", argv[0]);
    }

    app_svr_config_t app_svr_config;
//	ret = init_app_svr_config(SVR_CONFIG_PATH, &app_svr_config);
    ret = init_app_svr_base_config(SVR_CONFIG_BASE_PATH, &app_svr_config, SVR_CONFIG_BASE);
    if (ret < 0) {
        err_exit("init configure error");
    }

	ret = init_app_svr_base_config(SVR_CONFIG_TSCE_INDEX_PATH, &app_svr_config, SVR_CONFIG_TSCE_INDEX);
    if (ret < 0) {
        err_exit("init configure error");
    }


    if ( (f = fopen(TEMP_STOP_RESULT_FILE, "a+")) < 0) {
        err_sys("open temp file %s failed", TEMP_STOP_RESULT_FILE);
        exit(1);
    }
    node_list_t *node_list = &(app_svr_config.node_list);
    cf_node_t *node = &(node_list->head);
    const int count = node_list->count;

    char cmd [2048];
    int i = 0;
    for (i = 0; i < count; i++) {
        if (node->next == NULL) {
            break;
        }
        printf("%s:\n", node->next->node_name); 
        sprintf(cmd, "ssh %s \"cd %s;./tscecd_stop\"", \
                node->next->node_name, argv[2]);
        ret = system(cmd);
        if (ret != 0) {
            fprintf(f, TEYE_CLIENT_STOP_FAILED, node->next->node_name);
        } else {
            fprintf(f, TEYE_CLIENT_STOP_SUCCESS, node->next->node_name);
        }
        node = node->next;
    }

    app_config_free_node_list(node_list);

    exit(0);
}


static int 
check_args(int argc, char *argv[])
{
    assert(argv != NULL);

    if (argc != 3 || strcmp(argv[1], "-d") != 0) {
        return -1;
    }
    return 0;
}
