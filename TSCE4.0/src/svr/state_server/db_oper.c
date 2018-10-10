#include <stdio.h>
#include "db_oper.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
//#include "log.h"

//#define DBCFG       "../gsmd.conf"
/*创建一个数据库连接*/
MYSQL* lk_conndb(char* IPAddr, char* DbUsr, char* DbPwd, char* DbName)
{
    MYSQL *conn_ptr = NULL;
    //hzw
    MYSQL *mysql_ptr = NULL;
    mysql_ptr = mysql_init(NULL);
    if (!mysql_ptr)
    {
        printf("mysql_init failed\n");
        return NULL;
    }

//    print_debug("&&&&&&&&&&&&&&&&%s========================\n",pfilename);
    printf("*** %s %s %s %s\n", DbName, IPAddr, DbUsr, DbPwd);
    if (!(conn_ptr = mysql_real_connect(mysql_ptr, IPAddr, DbUsr, DbPwd, DbName, 0, NULL, 0)))
    {
        mysql_close(mysql_ptr);
        printf("return connection error\n");
        return NULL;
    }

    printf("connect success\n");
    return conn_ptr;
}


/*
 * added by fuchencong 2014.12.11
 */
int
check_conn(MYSQL** conn)
{
	assert(conn != NULL && *conn != NULL);
	
	if (0 == mysql_query(*conn, CHECK_STATEMENT)){
		MYSQL_RES *res;
		res = mysql_store_result(*conn);
		mysql_free_result(res);
		return 0;
	}
	else{
		printf("check error: ERROR %u (%s): %s\n", mysql_errno(*conn),
					mysql_sqlstate(*conn), mysql_error(*conn));
	
		/*
         * CR_SERVER_GONE_ERROR returns if connection is timeout
         * or service mysqld stop, or other reasons
         */

/*
		if (CR_SERVER_GONE_ERROR == mysql_errno(*conn)){
			MYSQL *new_conn = lk_conndb(pcfg_info.IPAddr, pcfg_info.DbUsr,
									    pcfg_info.DbPwd, pcfg_info.DbName);		
			if (new_conn){
				mysql_close(*conn);
				*conn  = new_conn;
				return 0;
			}
		}		
*/		return -1;
	}	
} 


//关闭一个数据库连接
int lk_closedb(MYSQL *conn_ptr)
{
    mysql_close(conn_ptr);
    return EXIT_SUCCESS;
}

//查询表信息函数,返回值为查询的结果
//参数说明：str_sql数据操作语句， conn_ptr数据库连接句柄， res_buf存放查询结果, buf_size为res_buf的大小
//返回值：查询失败返回NULL，查询成功返回res_buf
char* select_res(char* str_sql, MYSQL* conn_ptr, char *res_buf, int buf_size)
{
    int i;
    int j;
    int ret;

    MYSQL_RES *res_ptr;
    MYSQL_ROW sqlrow;

    res_ptr = NULL;
    memset(res_buf, 0, buf_size);
    if (conn_ptr)
    {
    	ret = mysql_query(conn_ptr, str_sql); //查询语句
        if (ret)
        {
            printf("SELECT error;%s\n", mysql_error(conn_ptr));

            return NULL;
        }
        else
        {
            res_ptr = mysql_store_result(conn_ptr);            //取出结果集
            if (res_ptr)
            {
                j = mysql_num_fields(res_ptr);
                while ((sqlrow = mysql_fetch_row(res_ptr)))
                {
                	//遍历查询结果，将结果复制到res_buf中
                	for (i = 0; i < j; i++)
                    {
                		//列值有可能为null， 暂时不启用，如有问题则启用
//                		if(sqlrow[i] && *sqlrow[i] != '\0')
//                		{
//                			if(strlen(res_buf) + strlen(sqlrow[i]) > buf_size + 2)
//                			{
//                				insert_log(LOG_MAIN, "result is longer than buf size");
//                				mysql_free_result(res_ptr);
//                				return NULL;
//                			}
//                			strcat(res_buf, sqlrow[i]);
//                		}
                		strcat(res_buf, sqlrow[i]);
                        sprintf(res_buf, "%s\t", res_buf);
                    }
                    sprintf(res_buf, "%s\n", res_buf);

                }
                if (mysql_errno(conn_ptr))
                {
                    printf("Retrive error;%s\n", mysql_error(conn_ptr));
                }
            }
            mysql_free_result(res_ptr);
        }
    }
    else
    {
        printf("Connection failed\n");

        return NULL;

    }

    if (strcmp(res_buf, "") == 0)
    {
        printf("result is null, str_sql:%s\n", str_sql);
        return NULL;
    }

    return res_buf;
}

//插入,删除,更新通用函数  返回操作的表的行数
unsigned long operate_res(char* str_sql, MYSQL* conn_ptr)       //插入函数
{
    int res;
    unsigned long num = 0;       //更新插入删除的行数
    if (!str_sql)
        return 0;
    if (conn_ptr)
    {
        char str[30];
        memset(str, 0, sizeof(str));
        strcpy(str, "set names \'utf8\'");
        if (mysql_real_query(conn_ptr, str, strlen(str)))
        {
            printf("mysql code is error\n");
            return 0;
        }

        res = mysql_real_query(conn_ptr, str_sql, strlen(str_sql));
        if (!res)
        {
            num = (unsigned long) mysql_affected_rows(conn_ptr);
        }
        else
        {
        	printf("Insert error %d; %s\n", mysql_errno(conn_ptr), mysql_error(conn_ptr));
            return 0;
        }
    }
    else
    {
        printf("Connection failed\n");
    }
    return num;
}

//
////连接池初始化
//Connpool* Connpool_init(int max_num,int min_num,int init_num)
//{
//    Connpool *pool=NULL;
//      pool=(Connpool*)malloc(sizeof(Connpool));
//
//      Conn *phead=NULL;
//      phead=(Conn*)malloc(sizeof(Conn));
//      phead->next=NULL;
//
//    int i=0;  //初始化init_num个连接
////    for(i=0;i<init_num;i++)
//    while(i<init_num)
//    {
//        Conn *pconn_tmp=NULL;
//        pconn_tmp=(Conn*)malloc(sizeof(pconn_tmp));
//        pconn_tmp->next=NULL;
//        pconn_tmp->conn_ptr=NULL;
//        MYSQL *conn_ptr=NULL;
//        conn_ptr=lk_conndb(DBCFG);
//        pconn_tmp->conn_ptr=conn_ptr;
//        if(pconn_tmp->conn_ptr)
//        {
//            pconn_tmp->flag=0;
//            //将该链接插入表尾
//            Conn *ptail=phead;
//            while(ptail->next)
//            {
//                ptail=ptail->next;
//            }
//            ptail->next=pconn_tmp;
//            i++;
//        }
//        else
//        {
//            free(pconn_tmp);
//        }
//    }
//    pool->pconn_head=phead;
//    pool->max_conn_num= max_num;
//    pool->cur_conn_num= init_num;
//    pool->min_conn_num= min_num;
//    pool->init_conn_num= init_num;
//    return pool;
//}
//
////连接池释放
//int Connpool_free(Connpool* pool)
//{
//     if(!pool)
//         return 0;
//     if(!pool->pconn_head)
//     {
//          free(pool);
//          pool=NULL;
//          return 0;
//     }
//     Conn* pconn_head=pool->pconn_head;
//     if(!pconn_head)
//         return 0;
//     while(pconn_head->next)
//     {
//          Conn* pconn=pconn_head->next;
//          pconn_head->next=pconn->next;
//          lk_closedb(pconn->conn_ptr);
//          free(pconn);
//     }
//     free(pconn_head);
//     pconn_head=NULL;
//     free(pool);
//     pool=NULL;
//     return 0;
//}
//
//
////获取连接
//MYSQL* GetConn(Connpool* pool)
//{
//    if(!pool)
//       return NULL;
//    if(!pool->pconn_head)
//       return NULL;
//
//    Conn *pconn=NULL;
//    pconn=pool->pconn_head;
//    //遍历连接，看是否有未用过的
//    while(pconn->next)
//    {
//        print_debug("ffffffffffffffffffffffffffffffff\n");
//        pconn=pconn->next;
//        //从头遍历
//        if(pconn->flag==0)
//        {
//            MYSQL *conn_ptr=NULL;
//            pconn->flag=1;
//            conn_ptr=pconn->conn_ptr;
//            return conn_ptr;
//        }
//    }
//    //没有遍历到，判断能否创建
//    //当前连接数小于最大连接数，创建一个连接
//    if(pool->max_conn_num>pool->cur_conn_num)
//    {
//        Conn *pconn_tmp=NULL;
//        pconn_tmp=(Conn*)malloc(sizeof(pconn_tmp));
//        pconn_tmp->next=NULL;
//        pconn_tmp->conn_ptr=NULL;
//        MYSQL *conn_ptr=NULL;
//        conn_ptr=lk_conndb(DBCFG);
//        pconn_tmp->conn_ptr=conn_ptr;
//        if(pconn_tmp->conn_ptr)
//        {
//            pconn_tmp->flag=0;
//            pconn->next=pconn_tmp;
//            pool->cur_conn_num++;
//            return pconn_tmp->conn_ptr;
//        }
//        else
//        {
//            free(pconn_tmp);
//            return NULL;
//        }
//    }
//    else
//       return NULL;
//}
//
////释放连接
//int FreeConn(MYSQL* conn_ptr,Connpool* pool)
//{
//     if(!pool)
//         return -1;
//     if(!conn_ptr)
//         return -1;
//     if(!pool->pconn_head)
//         return -1;
//     Conn* pconn=NULL;
//     pconn=pool->pconn_head;
//     while(pconn->next)
//     {
//          pconn=pconn->next;
//          if((pconn->flag==1)&&(pconn->conn_ptr==conn_ptr))
//          {
//                pconn->flag=0;
//                return 0;
//          }
//     }
//     //没找到，返回1
//     return 1;
//}
//
////连接池管理
//int Connpool_manage(Connpool* pool,int max_freeconn,int min_freeconn)
//{
//    int  freeconn_num=0;
//    if(!pool)
//        return -1;
//    if((max_freeconn==0)||(min_freeconn)==0)
//        return -2;
//    if(!pool->pconn_head)
//        return -3;
//    //如果当前连接数>=最大连接数
//    if(pool->cur_conn_num>=pool->max_conn_num)
//          return 1;
//    Conn *pconn=pool->pconn_head;
//
//    //查询空闲连接数
//    while(pconn->next)
//    {
//         pconn=pconn->next;
//         if(pconn->flag==0)
//              freeconn_num++;
//    }
//    //如果小于最小空闲数，添加连接
//    if(freeconn_num<min_freeconn)
//    {
//        int num=min_freeconn-freeconn_num;             //需要添加的空闲连接数
//         int i=0;
//         while((pool->cur_conn_num<pool->max_conn_num)&&(i<num))
//         {
//              Conn *pconn_tmp=NULL;
//              pconn_tmp=(Conn*)malloc(sizeof(pconn_tmp));
//              pconn_tmp->next=NULL;
//              pconn_tmp->conn_ptr=NULL;
//              MYSQL *conn_ptr=NULL;
//              conn_ptr=lk_conndb(DBCFG);
//              pconn_tmp->conn_ptr=conn_ptr;
//              if(pconn_tmp->conn_ptr)
//              {
//                  pconn_tmp->flag=0;
//                  pconn->next=pconn_tmp;
//                  pool->cur_conn_num++;
//                  pconn=pconn->next;
//                  i++;
//              }
//              else
//              {
//                  free(pconn_tmp);
//              }
//         }
//    }
//    //如果大于最大空闲数，从标头开始释放部分连接
//    if(freeconn_num>max_freeconn)
//    {
//         int num=freeconn_num-max_freeconn;
//         Conn *pconn_free=pool->pconn_head;
//         int i=0;
//         while((pconn_free->next)&&(i<num))
//         {
//              if((pconn_free->next->flag==0))
//              {
//                  Conn *pconn_tmp=NULL;
//                  pconn_tmp=pconn_free->next;
//                  pconn_free->next=pconn_tmp->next;
//                  lk_closedb(pconn_tmp->conn_ptr);
//                  free(pconn_tmp);
//                  pool->cur_conn_num--;
//                  i++;
//              }
//              else
//              {
//                  pconn_free=pconn_free->next;
//              }
//         }
//    }
//    return 0;
//}

//int main()
//{
//     Connpool *pool=NULL;
//     pool=Connpool_init(10,5,6);
//     if(!pool)
//         return -1;
//     MYSQL* conn_ptr=GetConn(pool);
//     print_debug("%s\n",select_res("select nodename from AlarmSendRecord limit 2",conn_ptr));
//     FreeConn(conn_ptr,pool);
//
//     Connpool_free(pool);
//     return 0;
////   Connpool_manage(Connpool* pool,8,3);
//}
