
/*
 * Copyright (C) Inspur(Beijing)
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "clr.h"
#include "clr_config.h"
#include "clr_define.h"
#include "error_handle.h"
#include "protocol.h"
#include "data.h"
#include "common_util.h"
#include "util.h"
#include "state_client.h"

/* max connect times */
#define MAX_CONNECT_TIMES 60


/* global module struct */
app_clr_module_t app_clr_module;
struct script script_t;

/* monitor list */
clr_monitor_list_t clr_monitor_list;

static int clr_init_enviroment();
static int init_module_array();
//static int monitor_data();


int 
main(int argc, char *argv[]) 
{
    int ret;
    char error_msg[MAX_ERROR_MESSAGE_LENGTH];
    app_clr_config_t app_clr_config;
    config_mod_info_t   mod_info_head, monitor_mod_info_head;
	

    bzero(&app_clr_module, sizeof(app_clr_module));
    bzero(&mod_info_head, sizeof(config_mod_info_t));
    bzero(&clr_monitor_list, sizeof(clr_monitor_list));
    bzero(&monitor_mod_info_head, sizeof(config_mod_info_t));

    /* check whether another process is running */
    ret = check_pid(CLR_PID_FILE_PATH);
    if (ret > 0) {
        err_exit("Another process (pid %d) is running", ret);
    }

    /* init enviroment */
    ret = clr_init_enviroment();
    if (ret < 0) {
        err_exit("init enviroment error: create directory error");
    }

    /* config */
    ret = init_app_clr_config(CLR_CONFIG_PATH, &app_clr_config);
    if (ret < 0) {
        err_exit("init client configure error");
    }

    /* index info */
    ret = analyse_index_info_config(BASE_INDEX_CONFIG_PATH, &mod_info_head);
    if (ret < 0) {
        err_exit("init tsce index configure file error");
    }

	/* monitor list */
	ret = init_monitor_list_config(TSCE_INDEX_CONFIG_PATH, &clr_monitor_list);
    if (ret < 0) {
        err_exit("init tsce index configure file error");
    }

    /* daemoize */

    pid_t daemonize = fork();
    if (daemonize < 0) {
        err_exit("Can't run as a daemon process");
    } else if (daemonize > 0) {
        free_index_info_config(&mod_info_head);
        exit(0);
    }
    umask(0); 
    setsid();
    ignore_all_signals();



    freopen ( "/dev/null", "r", stdin );
    freopen ( "/dev/null", "w", stdout );
    freopen ( "/dev/null", "w", stderr );


    /* log */
    cs_log_start(app_clr_config.log_level);

    /* record pid */
    FILE * pid_file = fopen(CLR_PID_FILE_PATH, "w");
    if (pid_file != NULL) {
        fprintf(pid_file, "%d", getpid());
        fclose(pid_file);
    }

	/* what will be monitor */
    monitor_data(&mod_info_head, &clr_monitor_list, &monitor_mod_info_head);
    init_module_array(&monitor_mod_info_head, &clr_monitor_list);
    /* load modules */
    load_modules();


    /* some config */ 
    const char * server_ip = app_clr_config.svr_config.svr_ip;
    const unsigned short int server_port = app_clr_config.svr_config.svr_port;
    const int INTERVAL = app_clr_config.itvl;

    int client_sockfd;
    struct sockaddr_in server_address;
    int server_len;

    /* server address */
    server_address.sin_family = AF_INET;
    ret = ipv4_pton(server_ip, &(server_address.sin_addr));
    server_address.sin_port = htons(server_port);
    server_len = sizeof(server_address);


    int connect_count = 0;

    pthread_t tid_ns;
    ret = pthread_create(&tid_ns, NULL,  handle_state, NULL);	
    if (ret != 0) {
	cs_log(LOG_ERROR, "%s", "create thread node state error" );
        goto FINISH_PROCESS;
    } else {
	cs_log(LOG_INFO, "%s", "create thread node state success");
    }
 
RECONNECT:


    if (connect_count == MAX_CONNECT_TIMES) {
        cs_log(LOG_ERROR, "TSCE server is down for %d s", connect_count);
//        cs_log(LOG_ERROR, "connect failed %d timess", connect_count);
//        goto FINISH_PROCESS;
    }

    /* socket */
    client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sockfd == -1) {
        strerror_r(errno, error_msg, MAX_ERROR_MESSAGE_LENGTH);
        cs_log(LOG_ERROR, "socket error errno is %d %s", errno, error_msg);
        goto FINISH_PROCESS;
    } else {
        cs_log(LOG_DEBUG, "socket success");
    }

    ret = set_socket_options(client_sockfd, 1, \
            CLR_SEND_TIMEOUT, CLR_RECV_TIMEOUT, \
            error_msg, MAX_ERROR_MESSAGE_LENGTH);
    if (ret < 0) {
        cs_log(LOG_ERROR, "setsockopt error: ret = %d, %s", ret, error_msg);
        goto FINISH_PROCESS;
    } else {
        cs_log(LOG_DEBUG, "setsockopt success");
    }

    ret = connect(client_sockfd, (struct sockaddr *)&server_address, \
            server_len);
    if (ret == -1) {
        strerror_r (errno, error_msg, MAX_ERROR_MESSAGE_LENGTH);
        cs_log(LOG_WARN, "connect error: errno is %d %s", errno, error_msg);
        close(client_sockfd);
        connect_count++;
        sleep(1);
        goto RECONNECT;
    } else {
        /* reset connect times */
        connect_count = 0;
        cs_log(LOG_INFO, "connect success");
    }

    /* package */
    app_pkg_t app_pkg;
    int app_pkg_len;
    app_hdr_t *app_head = &app_pkg.head;
    app_data_t *app_data = &app_pkg.body;
    char recv_buffer[sizeof(app_hdr_t) + 32];
    app_hdr_t *recv_response = (app_hdr_t *)recv_buffer;
    memset(&app_pkg, 0, sizeof(app_pkg));

    /* handshake */
    /* send */
    app_head->request_type = REQUEST_TYPE_POST;
    SET_HELO(app_pkg.head);//head == HE;/////////////////////////////////////////////////////////////////////////////
    ret = gethostname((char *) app_data, MAX_NODE_NAME_LEN);
    if (ret == -1) {
        cs_log(LOG_ERROR, "get host name error");
        goto FINISH_PROCESS;
    }
    app_head->len = strlen((char *) app_data) + 1;

    /* ignore return value */
    send(client_sockfd, &app_pkg, sizeof(app_hdr_t) + app_head->len, MSG_NOSIGNAL);

    int recv_count = 0;
    while (1) {
        /* receive */
        int recv_len = recv(client_sockfd, (void *)app_head, sizeof(app_hdr_t), 0);
        if (recv_len == -1) {
            strerror_r(errno, error_msg, MAX_ERROR_MESSAGE_LENGTH);
            if (errno == EINTR) {
                cs_log (LOG_ERROR, "%s", \
                        "recv data error, interupted by signal" \
                        "continue recv data after 1 second" );
                if (++recv_count == RECV_RETRY_TIMES) {
                    cs_log(LOG_ERROR, "recv handshake data error" \
                            "have retry %d times", RECV_RETRY_TIMES);
                    goto FINISH_PROCESS;
                }
                sleep(1);
            } else {
                cs_log(LOG_ERROR, "recv data error: errno is %d %s", errno, error_msg);
                goto FINISH_PROCESS;
            } 

        } else if (recv_len == 0) {
            cs_log(LOG_ERROR, "connection is closed by server");
            goto FINISH_PROCESS;

        } else {
            cs_log(LOG_INFO, "handshake success");
            break;
        }
    }

    /* post data */
    produce_app_pkg_head(app_head);
    ret = begin_collect_data();

    while (1) {
        struct timeval start_time = timeval_current();
        ret = get_app_data();
	if (ret != 0){
		cs_log(LOG_ERROR, "script read error");
		goto FINISH_PROCESS;
	}

        memset(app_data, 0, sizeof(app_data_t));
        /* comple appfeature data, fill corresponding error mask */
//        complete_app_data(app_data, &mod_info_head);
        complete_app_data(app_data, &monitor_mod_info_head);
		
	script_t.script_j = 0;//make sure script be count from 0;
	printf("app data is:%s\n",app_data->buffer);
        /* send */
        app_pkg_len = sizeof(app_hdr_t) + strlen(app_data->buffer) + 1;
        ret = send(client_sockfd, &app_pkg, app_pkg_len, MSG_NOSIGNAL);
        if (ret == -1) {
            strerror_r(errno, error_msg, MAX_ERROR_MESSAGE_LENGTH);
            cs_log(LOG_ERROR, "send fail: errno is %d %s", errno, error_msg );
            close(client_sockfd);
            sleep(1);
            goto RECONNECT;
        } else if (ret < app_pkg_len) {
            cs_log(LOG_ERROR, "send data incomplete ret is %d less than %d", \
                    ret, app_pkg_len);
        } else {
            cs_log(LOG_INFO, "send data success");
        }

        /* receive */
        ret = recv(client_sockfd, (void *)recv_response, sizeof(app_hdr_t), 0);
        if (ret == -1) { 
            strerror_r (errno, error_msg, MAX_ERROR_MESSAGE_LENGTH);
            cs_log(LOG_ERROR, "recv fail: errno is %d, %s", errno, error_msg);
            close(client_sockfd);
            sleep(1);
            goto RECONNECT;
        } else if (ret == 0) {
            cs_log(LOG_ERROR, "svr has closed socketfd");
        } else if (ret == sizeof(app_hdr_t) && \
            recv_response->data_type == RESPONSE_POST_OK) {
            cs_log(LOG_INFO, "get response ok");
        } else {
            cs_log(LOG_ERROR, "get resopnse error");
        }

        smart_sleep(INTERVAL, &start_time);
    }


FINISH_PROCESS:
    free_modules();
    pthread_cancel(tid_ns);
    free_index_info_config(&mod_info_head);
    close(client_sockfd);
    cs_log_end;
    exit(0);
}


static int 
clr_init_enviroment () 
{
    int ret;

    ret = util_mkdir(APP_CLR_RUN_DIR); 
    if (ret < 0) { 
        err_msg("create directory \"%s\" error", APP_CLR_RUN_DIR);
        return -1;
    }
    ret = util_mkdir(APP_CLR_LOG_DIR);
    if (ret < 0) {
        err_msg("create directory \"%s\" error", APP_CLR_LOG_DIR);
        return -1;
    }
	 ret = util_mkdir(TMP_DATA);
    if (ret < 0) {
        err_msg("create directory \"%s\" error", TMP_DATA);
        return -1;
    }
    return 0;
}


static int
init_module_array(config_mod_info_t *mod_info_head, clr_monitor_list_t *monitor_list)
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


