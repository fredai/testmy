/********************************************************************
版权所有:浪潮（北京）电子信息技术有限公司
项目名称:服务器底层监控平台
版    本:V1.0
操作系统:Red Hat Enterprise Linux Server release 5.5
文件名称:listIP.h
文件描述:单向链表管理iP，动态存放IP，包括添加IP和节点状态。删除指定IP等函数
项 目 组:高性能服务器产品部
程 序 员:段国栋
程序目录：/root/dgd/c_test/ifnet/listIP.h
发布日期:2013年01月18日
修    订:
修改日期:2013年03月7日 增加了通用数据库链接，包括初始化，有查询，修改，删除
*********************************************************************/

#define STR_SIZE 16 //ip地址
#define MAX_IP_COUNT 7

#define MAX_FDS                 1024
#define MAX_EVENTS              1024
#define BACKLOG                 1024 
 /**
 节点的数据结构
 num 表示节点的状态，状态分为：0:正常监控 1:监控信息获取不到也表示代理没有启动 2:正常关机 3:网络不通
 str 表示节点IP
 */
 struct node
{
    char ip[16];
    int fd;
    int flag;
    struct node *next;
};

 /**
 插入指定状态的节点IP
 */
 struct node *insert (int sockfd, int flag);
 
 /**
 删除指定IP的节点
 */
 struct node *delet ();
 
 /**
 输出节点信息
 */
 void printhead ();
 int pingcommand (char *ipset);
 
void* thread_ns (void* argv);
