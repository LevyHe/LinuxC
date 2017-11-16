// 
// 创建人： levy
// 创建时间：Jun 5, 2017
// 功能：mysql_pool.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Jun 5, 2017                  levy          初版
// 

#include "mysql_pool.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
static mysql_pool pool_mysql;//连接池定义
unsigned int query_times = 0;//mysql查询所有的次数，用于测试
unsigned int res_count = 0;
static void set_abswaittime(struct timespec*outtime, int ms)
{
	long sec ;
	long usec ;
	struct timeval tnow;
	gettimeofday(&tnow, NULL);
	usec = tnow.tv_usec + ms*1000;
	sec =  tnow.tv_sec+usec/1000000;
	outtime->tv_nsec=(usec%1000000)*1000;
	outtime->tv_sec=sec;

}

/*创建一个新的mysql连接
 * @return NULL代表创建失败。
 */
mysql_conn * mysql_new_connection()
{
	mysql_conn * conn=malloc(sizeof(mysql_conn));
	conn->is_autocommit_off = 0;
	conn->err_flag = 0;
	if(mysql_init(&conn->conn)==NULL)
	{
		printf("can not init mysql: [%s]\n",strerror(errno));
		free(conn);
		return NULL;
	}
	if(mysql_options(&conn->conn, MYSQL_SET_CHARSET_NAME, "UTF8")!=0)
	{
		printf("can not set mysql options[errno = %d]: [%s]\n",mysql_errno(&conn->conn),mysql_error(&conn->conn));
		free(conn);
		return NULL;
	}
	//连接到mysql服务端，设置CLIENT_MULTI_STATEMENTS通知服务器客户端可以处理由多语句或者存储过程执行生成的多结果集
	if(mysql_real_connect(&conn->conn,pool_mysql.host,pool_mysql.username,pool_mysql.passwd,pool_mysql.database,pool_mysql.s_port,NULL, CLIENT_MULTI_STATEMENTS)==NULL)
	{
		printf("can not connect  mysql server[errno = %d]: [%s]\n",mysql_errno(&conn->conn),mysql_error(&conn->conn));
		free(conn);
		return NULL;
	}
	conn->next = NULL;
	conn->prev = NULL;
	return conn;
}

/*向链接池中压入一个mysql连接conn
 * */
void conn_push(mysql_conn * conn)
{
	mysql_conn *lc = pool_mysql.mysql_list;
	if(lc==NULL)
	{
		pool_mysql.mysql_list=conn;

	}else
	{
		while(lc->next)
		{
			lc=lc->next;
		}
		lc->next = conn;
		conn->prev = lc;
	}
	pool_mysql.free_connections++;
}

/*从连接池中出栈一个mysql连接
 * @return NULL表示连接池中没有可用的连接
 * */
mysql_conn * conn_pop()
{
	mysql_conn*conn = pool_mysql.mysql_list;
	if(conn != NULL)
	{

		pool_mysql.mysql_list = conn->next;
		if(pool_mysql.mysql_list)
		{
			pool_mysql.mysql_list->prev = NULL;
		}
		pool_mysql.free_connections--;
	}
	return conn;
}

/*初始化mysql连接池*/
void mysql_pool_init()
{
	mysql_conn * conn;
	strncpy(pool_mysql.host,"localhost",sizeof(pool_mysql.host));
	strncpy(pool_mysql.username,"vdgs",sizeof(pool_mysql.username));
	strncpy(pool_mysql.passwd,"vdgs",sizeof(pool_mysql.passwd));
	strncpy(pool_mysql.database,"pbbms",sizeof(pool_mysql.database));
	pool_mysql.s_port = 3306;
	pool_mysql.max_connections=MAX_KEPP_CONNECTIONS;
	pool_mysql.free_connections = 0;
	pool_mysql.mysql_list = NULL;
	pool_mysql.is_idle_block = 0;
	pthread_mutex_init(&pool_mysql.lock,NULL);
	pthread_cond_init(&pool_mysql.idle_signal,NULL);
	pthread_mutex_lock(&pool_mysql.lock);
	for(int i=0;i<pool_mysql.max_connections;i++)
	{
		conn=mysql_new_connection();
		if(conn)
		{
			conn_push(conn);
		}else
		{

		}

	}
	pthread_mutex_unlock(&pool_mysql.lock);
}

/*从连接池中获取一个mysql连接*/
mysql_conn * get_mysql_connection()
{
	pthread_mutex_lock(&pool_mysql.lock);
	mysql_conn *conn = conn_pop();
	if(conn)
	{
		query_times++;
		pool_mysql.used_connections++;
	}
	pthread_mutex_unlock(&pool_mysql.lock);
	return conn;
}

/*从连接池中获取一个mysql连接，如果没有可用连接测阻塞*/
mysql_conn * get_mysql_connection_block()
{
	struct timespec tp;
	pthread_mutex_lock(&pool_mysql.lock);
	mysql_conn *conn = conn_pop();
	while(conn == NULL)
	{
		pool_mysql.is_idle_block ++;
		set_abswaittime(&tp,5000);//设置超时等待时间
		if(pthread_cond_timedwait(&pool_mysql.idle_signal,&pool_mysql.lock,&tp)==ETIMEDOUT)
		{
			conn=NULL;
			break;
		}
		conn = conn_pop();
		pool_mysql.is_idle_block --;
	}
	if(conn)
	{
		pool_mysql.used_connections++;
		query_times++;
	}
	pthread_mutex_unlock(&pool_mysql.lock);
	return conn;
}

/*回收一个mysql连接到连接池*/
void release_mysql_connection(mysql_conn *conn)
{
	pthread_mutex_lock(&pool_mysql.lock);
	conn->next = NULL;
	conn->prev = NULL;
	conn_push(conn);
	pool_mysql.used_connections--;
	if(pool_mysql.is_idle_block)
	{
		pthread_cond_signal(&pool_mysql.idle_signal);
	}
	pthread_mutex_unlock(&pool_mysql.lock);
}

/*销毁一个mysql连接，并释放其占用的资源*/
void destory_mysql_connection(mysql_conn *conn)
{
	mysql_close(&conn->conn);
	free(conn);
}

//销毁连接池中的所有连接
void destory_mysql_pool()
{
	mysql_conn *conn;
	pthread_mutex_lock(&pool_mysql.lock);
	conn = conn_pop();
	for(;conn;conn = conn_pop())
	{
		destory_mysql_connection(conn);
	}
	pthread_mutex_unlock(&pool_mysql.lock);
}

/*从连接池中自动获取一个可用连接，并执行sql命令，只可以用来处理单条的sql语句，否则可能无法获取正确的结果
 * @flag 用于sql语句执行结果的状态放回
 * @return 如果sql语句执行正常返回执行结果，其它返回NULL
 * */
MYSQL_RES* mysql_execute_query(const char *sql,unsigned long length,int *flag)
{
	int res;
	MYSQL_RES *res_ptr;
	mysql_conn*con = get_mysql_connection_block();
	*flag=0;
	if(con == NULL)
	{
		*flag =-1;
		return NULL;
	}
	res=mysql_real_query(&con->conn,sql,length);
	if(res==0)
	{
		res_ptr = mysql_store_result(&con->conn);
		res = mysql_next_result(&con->conn);
	}
	while(res==0)
	{
		mysql_free_result(mysql_store_result(&con->conn));
		res = mysql_next_result(&con->conn);
	}
	Mysql_QueryEnd(con,res);
	*flag = res;
	return res_ptr;
}

/*处理无返回类型的sql语句，可以同时处理多条sql语句
 * @sql,@length 参考mysql_real_query()
 * @return  参考 mysql_pool错误列表
 * */
int mysql_none_query(const char *sql,unsigned long length)
{
	int res;
	MYSQL_RES *res_ptr;
	mysql_conn*con = get_mysql_connection_block();
	if(con == NULL)
	{
		return -1;
	}
	res=mysql_real_query(&con->conn,sql,length);
	while(res==0)
	{
		res_ptr = mysql_store_result(&con->conn);
		if(res_ptr)
		{
			mysql_free_result(res_ptr);
		}
		res = mysql_next_result(&con->conn);
	}

	Mysql_QueryEnd(con,res);
	return res;
}
/*使用事务来处理大批量的sql插入操作，提高插入速度*/
int mysql_commit_query(const char *sql,unsigned long length)
{
	int res;
	MYSQL_RES *res_ptr;
	mysql_conn*con = get_mysql_connection_block();
	if(con == NULL)
	{
		return -1;
	}
	mysql_autocommit(&con->conn,0);//关闭自动提交事务
	res=mysql_real_query(&con->conn,sql,length);
	while(res==0)
	{
		res_ptr = mysql_store_result(&con->conn);
		if(res_ptr)
		{
			mysql_free_result(res_ptr);
		}
		res = mysql_next_result(&con->conn);
		res_count++;
	}

	//Mysql_QueryEnd(con,res);
	mysql_commit(&con->conn);		//提交事务
	mysql_autocommit(&con->conn,1);//再次开启自动提交事务
	release_mysql_connection(con);
	return res;
}

/*开启mysql事务查询，自动提交事务将被关闭
 * @return 0 成功返回,-1 无法获取mysql 链接，1关闭自动提交事务失败*/
int mysql_query_start(mysql_conn**con_ptr)
{
	int res;
	mysql_conn*con = get_mysql_connection_block();
	if(con == NULL)
	{
		return -1;
	}
	*con_ptr = con;
	res=mysql_autocommit(&con->conn,0);	//关闭自动提交事务
	con->is_autocommit_off = !res;
	return res;
}

/*结束mysql事务处理，并且开启自动提交事务
 * @is_rokkback 1 回滚事务，0提交事务
 *
 * */
int mysql_query_end(mysql_conn *con,int is_rokkback)
{
	int res;
	if(con->is_autocommit_off)
	{
		if(is_rokkback)
		{
			res=mysql_rollback(&con->conn);//处理错误，回滚操作
		}else
		{
			res=mysql_commit(&con->conn);//提交事务
		}
		con->is_autocommit_off=mysql_autocommit(&con->conn,1);//开启自动提交事务
	}
	if(res)
	{
		res=2;							//事务回滚或提交错误
		destory_mysql_connection(con);	//释放mysql链接所占用的资源
		con=mysql_new_connection();		//创建新的mysql链接
		if(con==NULL)
		{
			res = -2;					//无法创建新的mysql链接，需要重启进程
		}
		else
		{
			release_mysql_connection(con);//把新建立的链接加入连接池
		}
	}else
	{
		release_mysql_connection(con);
	}

	return res;
}



