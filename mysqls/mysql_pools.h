// 
// 创建人： levy
// 创建时间：Jun 5, 2017
// 功能：mysql_pool.h
// Copyright (c) 2017 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Jun 5, 2017                  levy          初版
// 


#ifndef MYSQL_POOLS_H_
#define MYSQL_POOLS_H_
#include <mysql.h>
#include <pthread.h>
#include <stdint.h>
#define DATABASE0_KEPP_CONNECTIONS 4
#define DATABASE1_KEPP_CONNECTIONS 4
#define DATABASE_NUMBERS 2

#define Mysql_QueryEnds(d_index,con,res)		do{\
if(res>0)\
	{\
		if(mysql_errno(&con->conn)>2000)\
		{\
			destory_mysql_connections(con);\
			con=mysql_new_connections(d_index);\
			if(con==NULL)\
			{\
				res = -2;\
			}\
			else\
			{\
				release_mysql_connections(d_index,con);\
			}\
		}else\
		{\
			res = 1;\
			release_mysql_connections(d_index,con);\
		}\
	}else\
	{\
		release_mysql_connections(d_index,con);\
		res=0;\
	}}while(0)

typedef struct mysql_conn		//mysql连接链表结构体定义
{
	struct mysql_conn * next;
	struct mysql_conn * prev;
	int is_autocommit_off;
	int err_flag;
	MYSQL conn;

}mysql_conn;

#define BoolValue(ptr) (int)(((ptr)==NULL)?0:(*(ptr)>1?atoi(ptr):*(ptr)))
#define IntValue(nptr)  (int)((nptr)==NULL?0:atoi(nptr))
#define FloatValue(nptr)  (double)((nptr)==NULL?0.0:atof(nptr))

typedef struct mysql_pool		//mysql连接池结构体定义
{
	char host[64];				//主机名称
	char username[32];			//用户名
	char passwd[32];			//密码
	char database[32];			//默认数据库
	int s_port;					//端口号，默认3306
	int max_connections;		//保持开启的mysql最大连接数
	int free_connections;		//当前空闲的mysql连接数
	int used_connections;		//正在使用的mysql连接数
	int is_idle_block;			//是否开启了无可用连接阻塞
	pthread_mutex_t lock;		//mysql链表锁
	pthread_cond_t idle_signal;	//等待可用连接的条件变量
	mysql_conn * mysql_list;	//mysql连接池链表
}mysql_pool;

void mysql_pool_inits(uint32_t d_index,const char* host,const char*username,const char*passwd,const char*database,int port,uint32_t keep_connections);
void destory_mysql_pools(uint32_t d_index);
void destory_mysql_connections(mysql_conn *conn);
void release_mysql_connections(uint32_t d_index,mysql_conn *conn);
mysql_conn * get_mysql_connections(uint32_t d_index);

MYSQL_RES* mysql_execute_querys(uint32_t d_index,const char *sql,unsigned long length,int *flag);
int  mysql_none_querys(uint32_t d_index,const char *sql,unsigned long length);
mysql_conn * get_mysql_connections(uint32_t d_index);
mysql_conn * get_mysql_connection_blocks(uint32_t d_index);
void release_mysql_connections(uint32_t d_index,mysql_conn *conn);
int mysql_commit_querys(uint32_t d_index,const char *sql,unsigned long length);
int mysql_query_starts(uint32_t d_index,mysql_conn**con_ptr);
int mysql_query_ends(uint32_t d_index,mysql_conn *con,int is_rokkback);
int get_mysql_connections_nums(uint32_t d_index);


#endif /* MYSQL_POOLS_H_ */
