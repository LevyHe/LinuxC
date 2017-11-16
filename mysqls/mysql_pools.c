// 
// 创建人： levy
// 创建时间：Jun 5, 2017
// 功能：mysql_pool.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Jun 5, 2017                  levy          初版
// 

#include "mysql_pools.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
static mysql_pool pool_mysql[DATABASE_NUMBERS];//连接池定义
//unsigned int query_times = 0;//mysql查询所有的次数，用于测试
//unsigned int res_count = 0;
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
mysql_conn * mysql_new_connections( uint32_t d_index)
{
	if(d_index >=DATABASE_NUMBERS)
	{
		printf("database index is over \n");
		return NULL;
	}
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
	if(mysql_real_connect(&conn->conn,pool_mysql[d_index].host,pool_mysql[d_index].username,pool_mysql[d_index].passwd,pool_mysql[d_index].database,pool_mysql[d_index].s_port,NULL, CLIENT_MULTI_STATEMENTS)==NULL)
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
void conn_pushs(uint32_t d_index ,mysql_conn * conn)
{
	if(d_index >=DATABASE_NUMBERS)
	{
		return;
	}
	mysql_conn *lc = pool_mysql[d_index].mysql_list;
	if(lc==NULL)
	{
		pool_mysql[d_index].mysql_list=conn;

	}else
	{
		while(lc->next)
		{
			lc=lc->next;
		}
		lc->next = conn;
		conn->prev = lc;
	}
	pool_mysql[d_index].free_connections++;
}

/*从连接池中出栈一个mysql连接
 * @return NULL表示连接池中没有可用的连接
 * */
mysql_conn * conn_pops(uint32_t d_index )
{
	if(d_index >=DATABASE_NUMBERS)
	{
		return NULL;
	}
	mysql_conn*conn = pool_mysql[d_index].mysql_list;
	if(conn != NULL)
	{

		pool_mysql[d_index].mysql_list = conn->next;
		if(pool_mysql[d_index].mysql_list)
		{
			pool_mysql[d_index].mysql_list->prev = NULL;
		}
		pool_mysql[d_index].free_connections--;
	}
	return conn;
}

/*初始化mysql连接池*/
void mysql_pool_inits(uint32_t d_index,const char* host,const char*username,const char*passwd,const char*database,int port,uint32_t keep_connections)
{
	if(d_index >=DATABASE_NUMBERS)
	{
		return ;
	}
	mysql_conn * conn;
	strncpy(pool_mysql[d_index].host,host,sizeof(pool_mysql[d_index].host));
	strncpy(pool_mysql[d_index].username,username,sizeof(pool_mysql[d_index].username));
	strncpy(pool_mysql[d_index].passwd,passwd,sizeof(pool_mysql[d_index].passwd));
	strncpy(pool_mysql[d_index].database,database,sizeof(pool_mysql[d_index].database));
	pool_mysql[d_index].s_port = port;
	pool_mysql[d_index].max_connections=keep_connections;
	pool_mysql[d_index].free_connections = 0;
	pool_mysql[d_index].mysql_list = NULL;
	pool_mysql[d_index].is_idle_block = 0;
	pthread_mutex_init(&pool_mysql[d_index].lock,NULL);
	pthread_cond_init(&pool_mysql[d_index].idle_signal,NULL);
	pthread_mutex_lock(&pool_mysql[d_index].lock);
	for(int i=0;i<pool_mysql[d_index].max_connections;i++)
	{
		conn=mysql_new_connections(d_index);
		if(conn)
		{
			conn_pushs(d_index,conn);
		}else
		{

		}
	}
	pthread_mutex_unlock(&pool_mysql[d_index].lock);
}

/*从连接池中获取一个mysql连接*/
mysql_conn * get_mysql_connections(uint32_t d_index)
{
	if(d_index >=DATABASE_NUMBERS)
	{
		return NULL;
	}
	pthread_mutex_lock(&pool_mysql[d_index].lock);
	mysql_conn *conn = conn_pops(d_index);
	if(conn)
	{
		//query_times++;
		pool_mysql[d_index].used_connections++;
	}
	pthread_mutex_unlock(&pool_mysql[d_index].lock);
	return conn;
}

int get_mysql_connections_nums(uint32_t d_index)
{
	if(d_index >=DATABASE_NUMBERS)
	{
		return -1;
	}
	mysql_conn *conn=pool_mysql[d_index].mysql_list;
	int count=0;
	while(conn)
	{
		conn=conn->next;
		count++;
	}
	return count;
}

/*从连接池中获取一个mysql连接，如果没有可用连接测阻塞*/
mysql_conn * get_mysql_connection_blocks(uint32_t d_index)
{
	if(d_index >=DATABASE_NUMBERS)
	{
		return NULL;
	}
	if(pool_mysql[d_index].mysql_list==NULL)
	{
		return NULL;
	}
	struct timespec tp;
	pthread_mutex_lock(&pool_mysql[d_index].lock);
	mysql_conn *conn = conn_pops(d_index);
	while(conn == NULL)
	{
		pool_mysql[d_index].is_idle_block ++;
		set_abswaittime(&tp,5000);//设置超时等待时间
		if(pthread_cond_timedwait(&pool_mysql[d_index].idle_signal,&pool_mysql[d_index].lock,&tp)==ETIMEDOUT)
		{
			conn=NULL;
			break;
		}
		conn = conn_pops(d_index);
		pool_mysql[d_index].is_idle_block --;
	}
	if(conn)
	{
		pool_mysql[d_index].used_connections++;
		//query_times++;
	}
	pthread_mutex_unlock(&pool_mysql[d_index].lock);
	return conn;
}

/*回收一个mysql连接到连接池*/
void release_mysql_connections(uint32_t d_index,mysql_conn *conn)
{
	if(d_index >=DATABASE_NUMBERS)
	{
		return ;
	}
	pthread_mutex_lock(&pool_mysql[d_index].lock);
	conn->next = NULL;
	conn->prev = NULL;
	conn_pushs(d_index,conn);
	pool_mysql[d_index].used_connections--;
	if(pool_mysql[d_index].is_idle_block)
	{
		pthread_cond_signal(&pool_mysql[d_index].idle_signal);
	}
	pthread_mutex_unlock(&pool_mysql[d_index].lock);
}

/*销毁一个mysql连接，并释放其占用的资源*/
void destory_mysql_connections(mysql_conn *conn)
{
	mysql_close(&conn->conn);
	free(conn);
}

//销毁连接池中的所有连接
void destory_mysql_pools(uint32_t d_index)
{
	if(d_index >=DATABASE_NUMBERS)
	{
		return ;
	}
	mysql_conn *conn;
	pthread_mutex_lock(&pool_mysql[d_index].lock);
	conn = conn_pops(d_index);
	for(;conn;conn = conn_pops(d_index))
	{
		destory_mysql_connections(conn);
	}
	pthread_mutex_unlock(&pool_mysql[d_index].lock);
}

/*从连接池中自动获取一个可用连接，并执行sql命令，只可以用来处理单条的sql语句，否则可能无法获取正确的结果
 * @flag 用于sql语句执行结果的状态放回
 * @return 如果sql语句执行正常返回执行结果，其它返回NULL
 * */
MYSQL_RES* mysql_execute_querys(uint32_t d_index,const char *sql,unsigned long length,int *flag)
{
	if(d_index >=DATABASE_NUMBERS)
	{
		*flag=-1;
		return NULL;
	}
	int res;
	MYSQL_RES *res_ptr=NULL;
	mysql_conn*con = get_mysql_connection_blocks(d_index);
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
	Mysql_QueryEnds(d_index,con,res);
	*flag = res;
	return res_ptr;
}

/*处理无返回类型的sql语句，可以同时处理多条sql语句
 * @sql,@length 参考mysql_real_query()
 * @return  参考 mysql_pool错误列表
 * */
int mysql_none_querys(uint32_t d_index,const char *sql,unsigned long length)
{
	if(d_index >=DATABASE_NUMBERS)
	{
		return -1;
	}
	int res;
	MYSQL_RES *res_ptr;
	mysql_conn*con = get_mysql_connection_blocks(d_index);
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

	Mysql_QueryEnds(d_index,con,res);
	return res;
}
/*使用事务来处理大批量的sql插入操作，提高插入速度*/
int mysql_commit_querys(uint32_t d_index,const char *sql,unsigned long length)
{
	if(d_index >=DATABASE_NUMBERS)
	{
		return -1;
	}
	int res;
	MYSQL_RES *res_ptr;
	mysql_conn*con = get_mysql_connection_blocks(d_index);
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
		//res_count++;
	}

	//Mysql_QueryEnd(con,res);
	mysql_commit(&con->conn);		//提交事务
	mysql_autocommit(&con->conn,1);//再次开启自动提交事务
	release_mysql_connections(d_index,con);
	return res;
}

/*开启mysql事务查询，自动提交事务将被关闭
 * @return 0 成功返回,-1 无法获取mysql 链接，1关闭自动提交事务失败*/
int mysql_query_starts(uint32_t d_index,mysql_conn**con_ptr)
{
	if(d_index >=DATABASE_NUMBERS)
	{
		return -1;
	}
	int res;
	mysql_conn*con = get_mysql_connection_blocks(d_index);
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
int mysql_query_ends(uint32_t d_index,mysql_conn *con,int is_rokkback)
{
	if(d_index >=DATABASE_NUMBERS)
	{
		return -1;
	}
	int res=0;
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
		destory_mysql_connections(con);	//释放mysql链接所占用的资源
		con=mysql_new_connections(d_index);		//创建新的mysql链接
		if(con==NULL)
		{
			res = -2;					//无法创建新的mysql链接，需要重启进程
		}
		else
		{
			release_mysql_connections(d_index,con);//把新建立的链接加入连接池
		}
	}else
	{
		release_mysql_connections(d_index,con);
	}

	return res;
}



