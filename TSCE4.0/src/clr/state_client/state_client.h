/*
 * Copyright (C) Inspur(Beijing)
 */

#include  <unistd.h>
#include  <sys/types.h>       /* basic system data types */
#include  <sys/socket.h>      /* basic socket definitions */
#include  <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include  <arpa/inet.h>       /* inet(3) functions */

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define LISNUM 2
#define MAXBUF 1024
int socket_result(int port);
void* handle_state(void* argv);


