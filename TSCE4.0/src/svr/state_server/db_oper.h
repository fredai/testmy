#ifndef DB_OPER
#define DB_OPER

#include <mysql/mysql.h>
#include <mysql/errmsg.h> 

#define MAX_CREATE_CONNECT_TRY  3
#define MAX_RES_SIZE 	1000000
#define CHECK_STATEMENT "select 1"

typedef struct conn_list
{
    MYSQL *conn_ptr;
    int flag;                                    //是否被占用  0非，1是被占用
    struct conn_list *next;
}Conn;

typedef struct
{
    Conn * pconn_head;                           //连接池
    int max_conn_num;                            //最大连接数
    int cur_conn_num;                            //当前池中连接数
    int min_conn_num;                            //最小连接数，就是连接池中还剩多少个连接的时候需要增加链接
    int init_conn_num;                           //初始链接数
}Connpool;

//数据库基本操作函数库
MYSQL* lk_conndb(char* IPAddr, char* DbUsr, char* DbPwd, char* DbName);     //打开数据库

/*
 * check if the connect is valid
 */
int check_conn(MYSQL** conn);

int lk_closedb(MYSQL *conn_ptr);                                       		//关闭数据库
//查询表信息函数,返回值为查询的结果
//参数说明：str_sql数据操作语句， conn_ptr数据库连接句柄， res_buf存放查询结果, buf_size为res_buf的大小
//返回值：查询失败返回NULL，查询成功返回res_buf
char* select_res(char* str_sql, MYSQL* conn_ptr, char *res_buf, int buf_size);
unsigned long operate_res(char* str_sql,MYSQL* conn_ptr);            		//增删改数据库

//连接池初始化
Connpool* Connpool_init(int max_num,int min_num,int init_num);

//连接池释放
int Connpool_free(Connpool* pool);


//获取连接
MYSQL* GetConn(Connpool* pool);

//释放连接
int FreeConn(MYSQL* conn_ptr,Connpool* pool);

//连接池管理
int Connpool_manage(Connpool* pool,int max_freeconn,int min_freeconn);

#endif

