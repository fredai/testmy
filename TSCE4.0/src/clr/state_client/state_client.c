#include "state_client.h"

int socket_result(int port)
{
    int sockfd, new_fd;
    socklen_t len;
    struct sockaddr_in my_addr, their_addr;
    unsigned int myport, lisnum;
    char buf[MAXBUF + 1];
    fd_set rfds;
    struct timeval tv;
    int retval, maxfd = -1;
    myport = port;
    lisnum = LISNUM;
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        printf("called socket fail\n");
        exit(1);
    }
    int on = 1;
    setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = PF_INET;
    my_addr.sin_port = htons(myport);
    my_addr.sin_addr.s_addr = INADDR_ANY;
  //  my_addr.sin_addr.s_addr = inet_addr(momip);
    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        printf("called bind fail\n");
        exit(1);
    }
    if (listen(sockfd, lisnum) == -1) {
        perror("listen");
        printf("called listen fail\n");
        exit(1);
    }
    while(1)
    {
        len = sizeof(struct sockaddr);
        if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &len)) == -1)
        {
            perror("accept");
            printf("called accept fail\n");
            exit(errno);
        }
        else
        {
            printf("server: got connection from %s, port %d, socket %d\n", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port), new_fd);
	    while (recv(new_fd, buf, sizeof(buf), 0) > 0) {
                  len = send(new_fd, "c", 1, 0);
					if (len > 0) {
//                      printf("recv %s\n", buf);
                      ;
					}
                   else
                   {
                        printf("消息发送失败！错误代码是%d:%s\n", errno, strerror (errno));
                        printf("消息发送失败！错误代码是%d:%s\n", errno, strerror (errno));
                        close(new_fd);
            	        break;
                   }
    	        }
                printf("recv msg failed from the server, errmsg: %s\n", strerror(errno));
                close(new_fd);
	  }
    }
    close(sockfd);
}

//void main()
void* handle_state(void* argv)
{
    unsigned short int port = 5002;


//    printf("_________ip=, ______port=%d\n", port);
    socket_result(port);
//    pthread_exit (NULL);
    return (void*) 0;
}
