/********************************************************************

版权所有:浪潮（北京）电子信息技术有限公司
项目名称:服务器底层监控平台
版    本:V1.0
操作系统:Red Hat Enterprise Linux Server release 5.5
文件名称:serconthread.c
文件描述:增加多线程操作：0:正常监控 1:监控信息获取不到也表示代理没有启动 2:正常关机 3:网络不通
项 目 组:高性能服务器产品部
程 序 员:段国栋
文件来源：netstatus.c  serconthreadtime.c
编译：gcc -o netstatus socketservercon.c list_ip.c netstatus.c
编译sql实例：gcc -m32 -o mysqltest mysqltest.c -I /usr/include/mysql -L /usr/lib/mysql -lmysqlclient -lz -lm
最后编译：gcc -m32 -o netstatus socketservercon.c list_ip.c netstatus.c -I /usr/include/mysql -L /usr/lib/mysql -lmysqlclient -lz -lm
加多线程编译：gcc -lpthread -m32 -o netstatus socketservercon.c list_ip.c serconthread.c -I /usr/include/mysql -L /usr/lib/mysql -lmysqlclient -lz -lm
多线程和频率和调试编译：gcc -lpthread -m32 -o serconthread socketservercon.c list_ip.c serconthread.c -I /usr/include/mysql -L /usr/lib/mysql -lmysqlclient -lz -lm -g
编译脚本：makenetstatus
程序目录：/root/dgd/c_test/ifnet/

发布日期:2013年02月17日
修    订:
修改日期:2013年 3月 10日 测试有问题，报段错误 ,重新建立serconthreadtime.c文件加以调试.该文件全部通过
修改日期:2013年 3月 12日 不加多线程问题全部修改完毕，此版本加入多线程调试
修改日期：2013年3月 22日 增加输出日期控制和判断程序执行时间
修改日期：2013年3月 26日 修改当出现异常时，不用程序exit 0退出，而改用return;
修改日期：2013年6月 08日 修改ping命令方式，改为popen管道方式
修改日期：2014年5月 07日 焦芬芳修改
                         1.将“threadcon”线程实现从函数里面提到函数外面
                         2.将“数据库状态更新”从函数“handle_MOMstate”中提取出来
修改日期：2014年5月 23日 焦芬芳修改
                            在原来业务逻辑不变的基础上，将代码整理
*********************************************************************/
#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <mysql/mysql.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include "db_oper.h"
#include "state_server.h"
//#include "log.h"
#include "u_log.h"
#include "svr_define.h"
#include "svr.h"

#define FAILURE  -1;
#define MAXBUF 1024

static struct node *head = NULL;
static  MYSQL *s_state_conn = NULL;		/* database handle */
/* the interval freq that get all calculating node state, unit microsecond */
static  unsigned int s_state_freq = 0;    

static db_config_ns_t db_config_ns_t_i;
/*
* Function:pingcommand 
* Description:ping ipset in order to check the node network
* Input:ipset: the node ip addr
* Output:non
* Return:the network is connected,return 0;else return -1 
*/
/*
int pingcommand (char *ipset)
{
    int err = -1;
    FILE *pfile;
    char line[100];
    char line1[100];
    char token[10];
    int i = 0;

    if(NULL == ipset)
    {
        print_debug("ipset is null\n");
        LOG(LOG_FATAL,"%s", "ipset is null\n");
        return err;
    }

    memset (line, 0, sizeof (line));
    memset (line1, 0, sizeof (line1));
    memset (token, 0, sizeof (token));
    char pingset[50 + 1024] = "ping -c 2 -i 0.1 -W 1 ";
    strcat (pingset, ipset);

    pfile = popen (pingset, "r");
    if (NULL == pfile)
    {
        perror ("::===::");
        LOG(LOG_FATAL, "popen %s fail\n", pingset);
    }
    else
    {
        if (fgets (line, 100, pfile) != NULL)
        {
            line[strlen (line) - 1] = 0;
            if (fgets (line1, 100, pfile) != NULL)
            {
                line1[strlen (line1) - 1] = 0;
                while (line1[i] != ' ')
                {
                    token[i] = line1[i];
                    i++;
                }
                if (strlen (token) != 0)
                {
                    print_debug ("token=%s\n", token);
                    if (strcmp (token, "64") == 0)
                    {
                        print_debug ("ping is ok!!!!!!!!!!!\n");
                        err = 0;
                    }
                    else
                    {
                        print_debug ("ping ip=%s is error!\n", ipset);
                    }
                }
            }
        }
        pclose (pfile);
    }
    
    return err;
}
*/


/*************************************************
Function:insert 
Description:insert the status infomation into the head of the list
Input: 
          pstr:   ip str
          n:        node status
Output:non
Return:head node 
*************************************************/

struct node *write_state_flag (int sockfd, int flag)
{
    struct node *tmp_ptr = head;

	printf("****************sockfd = %d\n", sockfd);
    while (tmp_ptr != NULL) {
        if (tmp_ptr->fd == sockfd) {
            tmp_ptr->flag = flag;
            if (0 != flag) {
                close(sockfd);
                tmp_ptr->fd = 0;
            }
            break;
        }
        tmp_ptr = tmp_ptr->next;
    }
    if (NULL == tmp_ptr) {
        return NULL;
    }
    return tmp_ptr;
}

int traverse_state_list(void)
{
    struct node *tmp_ptr = head;

    while (tmp_ptr != NULL) {
        if (-1 == tmp_ptr->flag) {
            break;
        }
        tmp_ptr = tmp_ptr->next;
    }
    if (NULL == tmp_ptr && tmp_ptr != head) {
        return 0;
    } else {
        return -1;
    }
}


/*************************************************
Function:delete_list 
Description:delete the list
Input:non
Output:non
Return:non 
*************************************************/
struct node *delete_list ()
{
    struct node *p;

    while(head)
    {
        p = head;
        head = head->next;
        printf("free(%p)\n", p);
        if (p->fd > 0) {
            close(p->fd);
        }
        free(p);
    }
    printf("head = %p\n", head);
    return head;
}

 /*************************************************
Function:printhead 
Description:print the list info, debug function
Input:head: the list head
Output:non
Return:non 
*************************************************/
void printhead ()
{
    printf ("***************\n");
    struct node *temp;
    temp = head;
    printf ("output strings:\n");
    while (temp != NULL)
    {
        printf("\n%d-----%s------%p\n", temp->flag, temp->ip, temp);
        temp = temp->next;
    }
    return;
}
  

/*************************************************
Function:update_node_state
Description:将以head为头的链表内容入库
Input:non
Output:non
Return:non 
*************************************************/
void update_node_state (void)
{
    struct node *temp;
    unsigned long rowcount = 0;
    char query[1024];

//    LOG_START(SVR_LOG_PATH ,LOG_DEBUG);
    /* connect database */
    if (-1 == check_conn (&s_state_conn)) {
        printf ("called check_conn failed!!!\n");
//        LOG(LOG_FATAL,"%s", "called check_conn failed!!!\n");
        return;
    }
    
    temp = head;
    
    
    while (temp != NULL) {      /* update NodeInfo state */
        rowcount = 0;
        sprintf (query, "update NodeInfo set state='%d' where ip='%s'", temp->flag, temp->ip);
        printf ("state===%d-----ip===%s\n", temp->flag, temp->ip);
        printf("%s\n", query);
        
        rowcount = operate_res (query, s_state_conn);
        if(0 == rowcount) {
            printf("%s  ERR\n", query);
        }
        temp = temp->next;
    }
}

unsigned int get_microsec()
{
    unsigned int microsecond = 0;
    int ret = 0;
    struct timeval curtime;
    curtime.tv_sec = 0;
    curtime.tv_usec = 0;

    ret = gettimeofday(&curtime, NULL);
    if (ret == -1) {
        perror("gettimeofday");
        return ret;
    }

    microsecond = curtime.tv_sec * 1000000  + curtime.tv_usec;

    return microsecond;
}


unsigned int gettimediff(struct timeval time)
{
    int ret = 0;
    struct timeval curtime;
    curtime.tv_sec = 0;
    curtime.tv_usec = 0;

    ret = gettimeofday(&curtime, NULL);
    if (ret == -1) {
        perror("gettimeofday");
        return ret;
    }
    
    if (curtime.tv_usec < time.tv_usec) {
       return ((curtime.tv_sec - 1 - time.tv_sec) * 1000000 + (curtime.tv_usec + 1000000 - time.tv_usec));
    } else {
       return ((curtime.tv_sec - time.tv_sec) * 1000000 + (curtime.tv_usec - time.tv_usec));
    }
}

int epoll_add(int epollfd, int sockfd)
{
    struct epoll_event event;
    int ret = 0;
    int err = 0;

    event.data.fd = sockfd;
    event.events = EPOLLIN;

//    LOG_START(SVR_LOG_PATH ,LOG_DEBUG);
    ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
    if (-1 == ret) {
        err = errno;
//        LOG(LOG_FATAL, "epoll_ctl errmsg:%s\n", strerror(err));
        return -1;
    }
    
    return 0;
}
static int  setnonblocking(int sock)
{
    int opts;
//    LOG_START(SVR_LOG_PATH ,LOG_DEBUG);
    opts=fcntl(sock, F_GETFL);
    if(opts<0) {
//        LOG(LOG_WARN, "fcntl(sock,GETFL) errmsg:%s\n", strerror(errno));
        return -1;
    }
    opts = opts | O_NONBLOCK;
    if(fcntl(sock, F_SETFL, opts)<0) {
//    	LOG(LOG_WARN, "fcntl(sock,SETFL,opts) errmsg:%s\n", strerror(errno));
    	return -1;
    }
    return 0;
}

void  sock_create(int epollfd)
{
        struct sockaddr_in  ip;
        struct node    *tmp = head;
        struct linger l_delay;
        char                          buf[2] = "s";
        int                              sockfd  = 0;
        int                              on = 1;
        int                              ret = 0;
        int                              err = -1;
       
//	LOG_START(SVR_LOG_PATH ,LOG_DEBUG);
        while(tmp != NULL) {
            if (0 == tmp->fd) {
                if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                    err = errno;
//                    LOG(LOG_FATAL, "socket errmsg:%s\n", strerror(err));
                    goto next;
                }
               
                setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on));

                memset(&l_delay, 0, sizeof(struct linger));
                l_delay.l_onoff = 0;
                setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &l_delay, sizeof(struct linger));
       
                bzero (&ip, sizeof (ip));
                ip.sin_family = AF_INET;
                ip.sin_port = htons (db_config_ns_t_i.db_server_port);
//                ip.sin_port = htons (pcfg_info.state_port);
                if (inet_aton (tmp->ip, (struct in_addr *) &ip.sin_addr.s_addr) == 0) {
//                    LOG(LOG_FATAL, "%s, called inet_aton fail\n", tmp->ip);
                    close (sockfd);
                    goto next;;
                }

//                printf("\n is connecting the claculating node %s......\n", tmp->ip);

                if (-1 == setnonblocking(sockfd)) {  /*set non-blocking*/
//                    LOG(LOG_FATAL, "%s", "setnonblocking fail\n");
                    close (sockfd);
                    goto next;
                }

                ret = connect(sockfd, (struct sockaddr *)&ip, sizeof(ip)); 
                err = errno;
                if (-1 == ret && EINPROGRESS == err){    /* add to epoll and node_state list */
		    if (-1 == epoll_add(epollfd, sockfd)) {
			close(sockfd);
//			LOG(LOG_FATAL,"%s", "epoll_add fail\n");
			goto next;
		    }
		     tmp->fd = sockfd;
		} else if (0 == ret) {
			tmp->fd = sockfd;
			tmp->flag = 0;
		} else {
//			LOG(LOG_FATAL, "connect errmsg:%s\n", strerror(err));
			close(sockfd);
		}
	} else {
            if (-1 == send(tmp->fd, buf, strlen(buf), 0)) { /* send msg to the calculate node */
                err = errno;
                if (EAGAIN != err && EWOULDBLOCK != err) {
//                    LOG(LOG_WARN, "send errmsg: %s\n", strerror(err));
                    close(tmp->fd);
                    tmp->fd = 0;
                    continue;   /* first close the sockfd and connect the new sockfd if send failed */
                }
            }
            printf("sockfd %d send %s\n", tmp->fd, buf);
	}
next:
            tmp = tmp->next;
        }

        return;
}

void close_sock(void)
{
    struct node *tmp_node = head;

    while (tmp_node != NULL) {
        if (tmp_node->fd > 0) {
            close(tmp_node->fd);
        }
        tmp_node = tmp_node->next;
    }
}

int epoll_sockfd(int epollfd)
{
    char sendbuf[2] = "s";
    char recvbuf[2] = "";
    int i = 0;
    int ret = 0;
    int  tmp_ret = 0;
    int err = 0;
    int len = sizeof(int);
    int opt = 0;
    int nread = -1;
    struct timeval starttime;
    unsigned int tmptime = 0;
    struct epoll_event events[MAX_EVENTS];
    struct sockaddr_in addr;
    
//    LOG_START(SVR_LOG_PATH ,LOG_DEBUG);
    if(-1 == gettimeofday(&starttime, NULL)) {
        return -1;
    }

    while ((tmptime = gettimediff(starttime)) < (s_state_freq)) {  /* epoll wait events */
        ret = epoll_wait(epollfd, events, MAX_EVENTS, 1000); /* timeout = 1000ms */
        if (-1 == ret) {
            err = errno;
//            LOG(LOG_FATAL, "epoll_wait errmsg:%s\n", strerror(err));
            close_sock();
            close(epollfd);
            return -1;
        } else if (0 == ret) {
            continue;
        }
        
        for (i = 0; i < ret; i++) {      /* insert state to the node state list*/
            if (events[i].events & EPOLLIN) {   
                if (0 == getsockopt(events[i].data.fd, SOL_SOCKET, SO_ERROR, &opt, (socklen_t *)&len)) {
			printf("getsockopt msg:%d\n", opt);
                    if (0 == opt) {
                        if (ioctl(events[i].data.fd, FIONREAD, &nread) < 0) {
                            printf("ioctl(sockfd, FIONREAD, &nread) failed!\n)");
                        } else {
                            if (nread > 0) {
                                memset(recvbuf, 0, sizeof(recvbuf));
                                recv(events[i].data.fd, recvbuf, sizeof(recvbuf), 0);
                                printf("nread = %d, recvbuf = %s\n", nread, recvbuf);
                            } else {
                                printf("connect epollin\n");
                            }
                        }
                        write_state_flag(events[i].data.fd, 0);    
                    } else if (ECONNREFUSED == opt || EPIPE == opt) {// refuse connect
                        write_state_flag(events[i].data.fd, 1); 
                    } else if (ENETUNREACH == opt ||ETIMEDOUT  == opt || EHOSTUNREACH == opt) { //net busy
                        write_state_flag(events[i].data.fd, 3);
                    } else {
                        write_state_flag(events[i].data.fd, opt);
                    }
                } else {
                    write_state_flag(events[i].data.fd, -2);
                }
            } else {
                printf("is not EPOLLIN\n");
            }
        }
        if (0 == traverse_state_list()) {
            break;
        }
    }
    return 0;
}

void remove_not_in_ip(void)
{
    struct node *tmp_node = head;
    struct node *del_node = NULL;
    struct node *prior = head;

    while (tmp_node != NULL) {
        if (tmp_node->flag != -1) {
            if (prior == head) {
                head = tmp_node->next;    
            } else {
                prior->next = tmp_node->next;
            }
            del_node = tmp_node;
            tmp_node = tmp_node->next;
            if (del_node->fd > 0) {
                close(del_node->fd);
            }
            free(del_node);
            continue;
        }
        prior = tmp_node;
        tmp_node = tmp_node->next;
    }
}

int get_ip_list(char *ip_result)
{
    char *result_tmp = NULL;
    char *p = NULL;
    struct node *tmp_node = NULL;

//    LOG_START(SVR_LOG_PATH ,LOG_DEBUG);
    if (NULL == ip_result) {
//        LOG(LOG_FATAL, "%s", "the argument ip_result is NULL!\n");
        return FAILURE;
    }
    
    if (-1 == check_conn(&s_state_conn)) {   /* check database fd*/
//        LOG(LOG_FATAL,"%s", "called  check_conn  failed\n");
        return FAILURE;
    }
        
    if (NULL == select_res ("select ip from NodeInfo", s_state_conn, ip_result, MAX_RES_SIZE)) {    /* query ip */
//        LOG(LOG_FATAL,"%s", "select ip from NodeInfo through called select_res function fail!\n");
        return FAILURE;
    }
   
    result_tmp = ip_result;
    while((p = strtok_r(NULL, "\t\n", &result_tmp)) ) {   /* put ip into node list */
        tmp_node = head;
        while (tmp_node != NULL) {  /* set flag -1 if find the ip in the node list */
            if (0 == strncmp(tmp_node->ip, p, strlen(p)>strlen(tmp_node->ip)?strlen(p):strlen(tmp_node->ip))) {
                tmp_node->flag = -1;
                break;
            }
            tmp_node = tmp_node->next;
        }
        
        if (NULL == tmp_node) {     /* malloc the new struct node if the ip is not exist */
            tmp_node = (struct node *)malloc(sizeof(struct node));
            if (NULL == tmp_node) {
                delete_list();    /* free all malloc struct node if malloc fail */
                return FAILURE;
            }
            tmp_node->flag = -1;
            tmp_node->fd = 0;
            strncpy(tmp_node->ip, p, STR_SIZE);
            
            if (NULL == head) {     /* insert the new ip into the node list */
                tmp_node->next = NULL;
                head = tmp_node;
            } else {
                tmp_node->next = head;
                head = tmp_node;
            } 
        }
    }
    
    remove_not_in_ip(); /* delete the node that the flag is not -1 */
   
        return 0;
}


/*************************************************
Function:handle_MOMstate
Description:get state of the computing nodes through the ping cmd
Input:non
Output:non
Return:non 
*************************************************/
void *thread_ns (void *argv)
{   
    char *select_result = NULL;		/* Save the queried results from the database */
    int j = 0;
    int  epollfd = 0;
    int err = 0;
    struct timeval starttime;
    unsigned int run_time = 0;
    db_config_ns_t_i = *(db_config_ns_t*)argv;//save the message of db;


//    LOG_START(SVR_LOG_PATH ,LOG_DEBUG);
    printf("handle_MOMstate start.......\n");
//    insert_log(LOG_STATE, "handle_MOMstate start.......\n");
//    LOG(LOG_INFO, "%s", "handle_MOMstate start.......");       

    for (j = 0; j < MAX_CREATE_CONNECT_TRY; j++) {
//        s_state_conn = lk_conndb (pcfg_info.IPAddr, pcfg_info.DbUsr, pcfg_info.DbPwd, pcfg_info.DbName);
        s_state_conn = lk_conndb (db_config_ns_t_i.db_server_ip,db_config_ns_t_i.db_username , db_config_ns_t_i.db_password, "tsce");
        if  (NULL == s_state_conn) {
//            sleep (pcfg_info.state_freq);
            sleep (3);
            continue;
        }
        break;
    }
    if (NULL == s_state_conn) {
            printf("call lk_conndb failed!\n");
//            LOG(LOG_FATAL,"%s", "call lk_conndb failed!\n");
            return ((void *) 0);
    }

    select_result = (char *)malloc(MAX_RES_SIZE);   /* save the queried result */
    if (NULL == select_result) {
        printf ("malloc fail\n");
//        LOG(LOG_FATAL,"%s", "malloc fail\n");
        return ((void *) 0);
    }

    epollfd = epoll_create(MAX_FDS);    /* poll all node connectiong state */
    if (-1 == epollfd) {
        err = errno;
//        LOG(LOG_FATAL, "epoll_create errmsg:%s\n", strerror(err));
        return ((void *) 0);
    }

     printf("malloc select_result_ADDR = %p\n", select_result);

     s_state_freq = 3* 1000000;
//     s_state_freq = pcfg_info.state_freq * 1000000;
    
    for ( ; ; ) {
    
        if (-1 == gettimeofday(&starttime, NULL)) {
            continue;
        }
        printhead (head);    /* put the node state list */
        
        if (-1 == get_ip_list(select_result)) {
            goto loop;
        }

        sock_create(epollfd);  /* As the fd of 0 to create sock and add to the epollfd*/

        if (-1 == epoll_sockfd(epollfd)) {
            goto loop;
        }
        
        /* printhead (head);*/    /* put the node state list */
        
        update_node_state ();   /* update state in the NodeInfo table */
        
loop:
        run_time = gettimediff(starttime);
//	printf("run_time = %d, starttime = %d\n", run_time, starttime);
        if (run_time >= s_state_freq) {
            continue;
        } else {
            printf("usleep %d\n", s_state_freq - run_time);
            usleep (s_state_freq - run_time);
        }
    }
    
    lk_closedb(s_state_conn);
    
    if (select_result != NULL) {
        printf("free select_result_ADDR = %p\n", select_result);
        free(select_result);
    }
    
    delete_list();
    
    return ((void *) 0);
}
