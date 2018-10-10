
/*
 * Copyright (C) Inspur(Beijing)
 */


#ifndef TEYE_SVR_H
#define TEYE_SVR_H


#define TABLE_NAME "DataInfo"
struct db_config_ns {
	char db_server_ip[16];
	unsigned short int db_server_port;
	char db_username[512];
	char db_password[512];
};

typedef struct db_config_ns db_config_ns_t;


#endif
