
/*
 * Copyright (C) Inspur(Beijing)
 */



#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <setjmp.h>
#include "protocol.h""


int 
main(int argc, char *argv[])
{


    /* create socket */
	int client_sockfd;
	struct sockaddr_in server_address;
	int ret;
	static app_pkg_t  data_package; 
	int connect_times = 5;
RECONNECT:
	client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	char *server_ip = "127.0.0.1";
    if (client_sockfd == -1) {
		perror("error\n");
	} else {
		printf("success\n");
    }

    /* server address */
    server_address.sin_family = AF_INET; 
    inet_pton (AF_INET, server_ip, &(server_address.sin_addr));
    server_address.sin_port = htons(5001);
    int server_len = sizeof(server_address);

    ret = connect(client_sockfd, (struct sockaddr *) \
            &server_address, server_len);
    if (ret == -1) {
        close(client_sockfd);
		perror("connect error");
		if(--connect_times > 0) {
			usleep(1000*200);
			close(client_sockfd);
			goto RECONNECT;
		}
    } else {
        printf("connect success");
    }

    int recv_len;
    app_hdr_t *get_head = &(data_package.head);
    app_data_t *get_body = &(data_package.body);

    /* handshake */
    /* send */
    get_head->request_type = REQUEST_TYPE_USER;
    SET_HELO(*get_head);
	printf("------%x-------\n", get_head->request_type);
    ret = send(client_sockfd, (void *)get_head, sizeof(app_hdr_t), MSG_NOSIGNAL);
    if (ret == -1) {
        close(client_sockfd);
		perror("send error\n");
    } else {
        printf("send handshake message success");
    }

    /* receive */
    recv_len = recv(client_sockfd, (void *)get_head, sizeof(app_hdr_t), 0);
    if (recv_len == -1) {
        close(client_sockfd);
		perror("recv error\n");
    } else if (recv_len == 0) {
        close(client_sockfd);
        perror("connection is closed by server");
    }

    /* send */
	/*
    ret = send (client_sockfd, (void *)(&data_package), snd_len, MSG_NOSIGNAL);
    if (ret == -1) {
        close(client_sockfd);
		perror("send error\n");
    } else {
        printf("send request success");
    }
*/

    /* receive */
/*
	while (1) {
		recv_len = recv(client_sockfd, (void *)(get_body -> buffer), \
                sizeof(get_body -> buffer), 0);
        if (recv_len == -1) {
            close(client_sockfd);
            perror( "recv data error");

        } else if (recv_len == 0) {
            close(client_sockfd);
            perror("connection was closed by server");
            exit(0);

        } else {
            printf("receive data success");
            // must give a resopnse to server /
            send(client_sockfd, (void*)get_head, sizeof(app_hdr_t), MSG_NOSIGNAL);
        }
    }
*/
    close(client_sockfd);
    exit(0);
}


