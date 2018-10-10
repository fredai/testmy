
/*
 * Copyright (C) Inspur(Beijing)
 */


#ifndef TEYE_SERVICE_H
#define TEYE_SERVICE_H


#include "svr_config.h"


#define SERVER_SOCKET_LISTEN_LEN 30
#define NETWORK_RECOVERY_RETRY_TIMES 1000
#define NETWORK_RECOVERY_RETRY_INTERVAL 5


int init_sockfd(const app_svr_config_t *app_svr_config);
int app_service(const app_svr_config_t * app_svr_config);
int register_signal_usr1();


#endif
