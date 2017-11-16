// 
// 创建人： levy
// 创建时间：Jun 5, 2017
// 功能：mysql_assist.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Jun 5, 2017                  levy          初版
// 
#if 1
#include "mysql_assists.h"
#include "mylog.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "cJSON.h"

/*从tab中的获取猎命为column的值value，只可以用来查询单行表
 * @value 包括四种类型(int,double,char*)
 * @return 查询结果
 * */
int mysql_get_tab_value(uint32_t index,const char * tab ,const char * column,void *value,size_t size)
{
	MYSQL_RES *res_ptr;
	MYSQL_ROW row;
	MYSQL_FIELD *field;
	int flag;
	size_t sql_size;
	sql_size= strlen(tab)+strlen(column)+128;
	char *sql = malloc(sql_size);
	if(sql == NULL)
	{
		return -3;//内存分配失败，重启进程
	}
	size_t num=snprintf(sql,sql_size, "SELECT %s FROM %s limit 0,1;", column,tab);
	res_ptr=mysql_execute_querys(index,sql,num,&flag);
	free(sql);
	if(res_ptr!=NULL)
	{
		my_ulonglong rownums = mysql_num_rows(res_ptr); //获取结果集中的行数
		if(rownums>0)
		{
			row = mysql_fetch_row(res_ptr);
			field = mysql_fetch_field(res_ptr);
		}
		if (field->type == MYSQL_TYPE_BIT)
		{
			*(int*)value = BoolValue(row[0]);
		}
		else if (field->type == MYSQL_TYPE_LONG || field->type == MYSQL_TYPE_SHORT || field->type == MYSQL_TYPE_TINY)
		{
			*(int*)value = IntValue(row[0]);
		}
		else if (field->type == MYSQL_TYPE_VAR_STRING)
		{
			if(row[0]!=NULL)
			{
				strncpy(value,row[0],size);
			}
		}
		else if (field->type == MYSQL_TYPE_DOUBLE || field->type == MYSQL_TYPE_FLOAT)
		{
			*(double*)value = FloatValue(row[0]);
		}
		mysql_free_result(res_ptr);	//释放结果集占用内存
	}

	return flag;
}
/*更新表格tab中column列的值
 * @type 要更新列的类型 1 字符串，2数字,其它也会按照字符串处理
 * @return 0 没有错误
 * */
int mysql_update_tab_value(uint32_t index,const char *tab,const char *column,int type,const char * val, ...)
{
	size_t sql_size ;
	size_t va_size;
	size_t num;
	int res;
	char * va_buf = malloc(128);
	char *sql;
	if(va_buf==NULL)
	{
		return -3;//内存分配失败，重启进程
	}
	va_list argp;
	va_start(argp, val);
	va_size=vsnprintf(va_buf, 128, val, argp);
	va_end(argp);
	sql_size = strlen(tab)+strlen(column)+va_size+128;
	sql = malloc(sql_size);
	if(sql == NULL)
	{
		free(va_buf);
		return -3;//内存分配失败，重启进程
	}
	if(type==1)	//待更新元素为字符串类型
	{
		num = snprintf(sql,sql_size,"update %s set %s='%s' limit 1;",tab,column,va_buf);
	}
	else if(type == 2)//待更新单元格为数据类型
	{
		num = snprintf(sql,sql_size,"update %s set %s=%s limit 1;",tab,column,va_buf);
	}
	else	//其它一律按照字符串类型更新
	{
		//mysql_real_escape_string();
		num = snprintf(sql,sql_size,"update %s set %s='%s' limit 1;",tab,column,va_buf);
	}
	res=mysql_none_querys(index,sql,num);
	free(va_buf);
	free(sql);
	return res;
}

int mysql_get_tab_rows(uint32_t index,const char *tab)
{
	MYSQL_RES *res_ptr;
	MYSQL_ROW row;
	int flag;
	int count;
	char sql[256];
	size_t num=snprintf(sql,256, "SELECT TABLE_ROWS FROM information_schema.TABLES where TABLE_NAME='%s';",tab);
	res_ptr=mysql_execute_querys(index,sql,num,&flag);
	if(res_ptr!=NULL)
	{
		my_ulonglong rownums = mysql_num_rows(res_ptr); //获取结果集中的行数
		if(rownums>0)
		{
			row = mysql_fetch_row(res_ptr);
			count= IntValue(row[0]);
		}else
		{
			count= -1;
		}
		mysql_free_result(res_ptr);	//释放结果集占用内存
	}else
	{
		count= -1;
	}
	return count;

}

int wnet_update_arm_node_list(wnet_arm301_define**list)
{
	int res,res1;
	char *sql;
	size_t sql_size = 1024;
	size_t num;
	mysql_conn*con=NULL;
	mysql_conn*rcon=NULL;
	if(*list == NULL)
	{
		return 0;
	}
	sql = malloc(sql_size);
	if(sql == NULL)
	{
		return -3;
	}
	res=mysql_query_starts(MYSQL_LOCAL_INDEX,&con);

	res1=mysql_query_starts(MYSQL_REMOTE_INDEX,&rcon);
	for(wnet_arm301_define * node = *list;node; node = node->next)
	{
		if(node->state ==0)
		{
			num=snprintf(sql,sql_size,"update hy_wnet_arm301_values set state=%d,update_time=now() where num_id=%d;",node->state,node->num_id);

		}else if(node->state == 1 || node->state ==3)
		{
			num=snprintf(sql,sql_size,"update hy_wnet_arm301_values set imp_value=%d,ain_value0=%d,ain_value1=%d,ain_value2=%d,"
					"ain_value3=%d,ain_value4=%d,ain_value5=%d,ain_value6=%d,ain_value7=%d,state=%d,update_time=now() where num_id=%d;",node->imp_value,
					node->ain_value[0],node->ain_value[1],node->ain_value[2],node->ain_value[3],node->ain_value[4],node->ain_value[5],node->ain_value[6],node->ain_value[7],node->state,node->num_id);
		}
		else if(node->state == 4)
		{
			node->state=3;
			num=snprintf(sql,sql_size,"insert into hy_wnet_arm301_values values(0,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,now());",node->node_id,node->imp_value,node->exp_value,
					node->ain_value[0],node->ain_value[1],node->ain_value[2],node->ain_value[3],node->ain_value[4],node->ain_value[5],node->ain_value[6],node->ain_value[7],
					node->aout_value[0],node->aout_value[1],node->aout_value[2],node->aout_value[3],node->state);
		}
		else
		{
			num=snprintf(sql,sql_size,"delete from  hy_wnet_arm301_values where num_id=%d;",node->num_id);
		}
		if(mysql_real_query(&con->conn,sql,num) == 0)
		{
			mysql_free_result(mysql_store_result(&con->conn));
		}
		else
		{
			ERR_LOG("update tag value sql error [%d] %s\n[%s]", mysql_errno(&con->conn),mysql_error(&con->conn),sql);
			break;
		}
		if(rcon)
		{
			if(mysql_real_query(&rcon->conn,sql,num) == 0)
			{
				mysql_free_result(mysql_store_result(&rcon->conn));
			}
			else
			{
				ERR_LOG("update remote tag value error [%d] %s\n[%s]", mysql_errno(&rcon->conn),mysql_error(&rcon->conn),sql);
				//break;
			}
		}
		if(node->state==1 || node->state==0)
		{
			num=snprintf(sql,sql_size,"insert into hy_wnet_arm301_historys values(0,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,now());"
					,node->node_id,node->imp_value,node->exp_value,node->ain_value[0],node->ain_value[1],node->ain_value[2],node->ain_value[3],
					node->ain_value[4],node->ain_value[5],node->ain_value[6],node->ain_value[7],node->aout_value[0],node->aout_value[1],node->aout_value[2],
					node->aout_value[3],node->state);
		}
		if(mysql_real_query(&con->conn,sql,num) == 0)
		{
			mysql_free_result(mysql_store_result(&con->conn));
		}
		else
		{
			ERR_LOG("insert tag value history error [%d] %s\n[%s]", mysql_errno(&con->conn),mysql_error(&con->conn),sql);
			//break;
		}
	}
	free(sql);
	con->err_flag=res;
	res=mysql_query_ends(MYSQL_LOCAL_INDEX,con,res);
	if(rcon)
	{
		rcon->err_flag=res1;
		if(mysql_query_ends(MYSQL_REMOTE_INDEX,rcon,res1)!=0)
		{
			ERR_LOG("mysql commit error [%d] %s", mysql_errno(&rcon->conn),mysql_error(&rcon->conn));
		}
	}
	return res;
}

int wnet_get_arm_node_list(wnet_arm301_define ** list)
{
	int flag=0;
	MYSQL_RES *res_ptr;
	MYSQL_ROW row;
	const char *sql = "select num_id,node_id,imp_value,exp_value,ain_value0,ain_value1,ain_value2,ain_value3,ain_value4,ain_value5,ain_value6,ain_value7,"
			"aout_value0,aout_value1,aout_value2,aout_value3,state from hy_wnet_arm301_values where node_id>0;";
	res_ptr=mysql_execute_querys(MYSQL_LOCAL_INDEX,sql,strlen(sql),&flag);
	if(res_ptr!=NULL)
	{
		if(flag==0)
		{
			my_ulonglong rownums = mysql_num_rows(res_ptr);
			for(int i=0;i<rownums;i++)
			{
				row = mysql_fetch_row(res_ptr);
				wnet_arm301_define * node = malloc(sizeof(wnet_arm301_define));
				if(node)
				{
					node->prev=NULL;
					node->next=NULL;
					node->num_id = IntValue(row[0]);
					node->node_id = IntValue(row[1]);
					node->imp_value=IntValue(row[2]);
					node->exp_value = IntValue(row[3]);
					node->ain_value[0]=IntValue(row[4]);
					node->ain_value[1]=IntValue(row[5]);
					node->ain_value[2]=IntValue(row[6]);
					node->ain_value[3]=IntValue(row[7]);
					node->ain_value[4]=IntValue(row[8]);
					node->ain_value[5]=IntValue(row[9]);
					node->ain_value[6]=IntValue(row[10]);
					node->ain_value[7]=IntValue(row[11]);
					node->aout_value[0]=IntValue(row[12]);
					node->aout_value[1]=IntValue(row[13]);
					node->aout_value[2]=IntValue(row[14]);
					node->aout_value[3]=IntValue(row[15]);
					node->state = IntValue(row[16]);
					LIST_PUSH(list,node,wnet_arm301_define);
				}
			}
		}
		mysql_free_result(res_ptr);	//释放结果集占用内存
	}

	return flag;
}

int get_param_information(int paramid,param_define *param)
{
	MYSQL_RES *res_ptr;
	MYSQL_ROW row;
	int flag;
	size_t sql_size=256;
	char *sql = malloc(sql_size);
	if(sql == NULL)
	{
		return -3;//内存分配失败，重启进程
	}
	size_t num=snprintf(sql,sql_size, "select param_id,parts_id,node_id,param_io,coefficient,offset_value,param_type FROM hy_wnet_params where param_id=%d limit 0,1;", paramid);
	res_ptr=mysql_execute_querys(MYSQL_LOCAL_INDEX,sql,num,&flag);
	free(sql);
	if(res_ptr!=NULL)
	{
		my_ulonglong rownums = mysql_num_rows(res_ptr); //获取结果集中的行数
		if(rownums>0)
		{
			row = mysql_fetch_row(res_ptr);
			param->param_id = IntValue(row[0]);
			param->parts_id = IntValue(row[1]);
			param->node_id = IntValue(row[2]);
			if(row[3]!=NULL)
			{
				strncpy(param->param_io,row[3],sizeof(param->param_io));
			}
			param->coefficient= FloatValue(row[4]);
			param->offset_value= FloatValue(row[5]);
			param->param_type = IntValue(row[6]);

		}
		mysql_free_result(res_ptr);	//释放结果集占用内存
	}else
	{
		param->param_id=0;
	}
	return flag;
}

int get_param_information_all(int *p_length,param_define **param_ptr)
{
	MYSQL_RES *res_ptr;
	MYSQL_ROW row;
	int flag;
	size_t sql_size=256;
	char *sql = malloc(sql_size);
	if(sql == NULL)
	{
		return -3;//内存分配失败，重启进程
	}
	size_t num=snprintf(sql,sql_size, "select a.param_id,parts_id,node_id,param_io,coefficient,offset_value,param_type,threshold_value,param_value from hy_wnet_params as a "
			"left join hy_wnet_param_values as b on a.param_id=b.param_id where a.isuse=1;");
	res_ptr=mysql_execute_querys(MYSQL_LOCAL_INDEX,sql,num,&flag);
	free(sql);
	if(res_ptr!=NULL)
	{
		my_ulonglong rownums = mysql_num_rows(res_ptr); //获取结果集中的行数
		if(rownums>0)
		{
			param_define*param=calloc(rownums,sizeof(param_define));
			*p_length=rownums;
			*param_ptr=param;
			for(int i=0;i<rownums;i++)
			{
				row = mysql_fetch_row(res_ptr);
				param[i].param_id = IntValue(row[0]);
				param[i].parts_id = IntValue(row[1]);
				param[i].node_id = IntValue(row[2]);
				if(row[3]!=NULL)
				{
					strncpy(param[i].param_io,row[3],sizeof(param[i].param_io));
				}
				param[i].coefficient= FloatValue(row[4]);
				param[i].offset_value= FloatValue(row[5]);
				param[i].param_type = IntValue(row[6]);
				param[i].threshold_value= FloatValue(row[7]);
				param[i].param_value= FloatValue(row[8]);

			}
			mysql_free_result(res_ptr);	//释放结果集占用内存
		}else
		{
			param_ptr=NULL;
		}
	}else
	{
		param_ptr=NULL;
	}
	return flag;
}

int update_set_param_value(int param_id,char*param_value)
{
	size_t sql_size=256 ;
	size_t num;
	int res;
	char *sql;
	sql = malloc(sql_size);
	if(sql == NULL)
	{
		return -3;//内存分配失败，重启进程
	}
	num = snprintf(sql,sql_size,"update hy_wnet_param_values set param_value=%s,update_time=now() where param_id=%d;",param_value,param_id);
	res=mysql_none_querys(MYSQL_LOCAL_INDEX,sql,num);
	if(res!=0)
	{
		ERR_LOG("update local hy_wnet_param_values error ,%s",sql);
		free(sql);
		return res;
	}
	num += snprintf(sql+num,sql_size-num,"insert into hy_wnet_param_set_historys values(0,%d,%s,now());",param_id,param_value);
	res=mysql_none_querys(MYSQL_REMOTE_INDEX,sql,num);
	if(res!=0)
	{
		ERR_LOG("update remote hy_wnet_param_set_historys error ,%s",sql);
	}
	free(sql);
	return res;
}

int update_arm301_value(int node_id,int value,char* name)
{
	size_t sql_size=256 ;
	size_t num;
	int res;
	char *sql;
	sql = malloc(sql_size);
	if(sql == NULL)
	{
		return -3;//内存分配失败，重启进程
	}
	num = snprintf(sql,sql_size,"update hy_wnet_arm301_values set %s=%d where node_id=%d;",name,value,node_id);
	res=mysql_none_querys(MYSQL_LOCAL_INDEX,sql,num);
	if(res!=0)
	{
		ERR_LOG("update_arm301_value error sql(%s)"),sql;
	}
	free(sql);
	return res;
}

int update_read_param_value(param_update_type** plist)
{
	int res,res1;
	char *sql;
	size_t sql_size = 512;
	size_t num;
	mysql_conn*con=NULL;
	mysql_conn*rcon=NULL;
	if(*plist == NULL)
	{
		return 0;
	}
	sql = malloc(sql_size);
	if(sql == NULL)
	{
		return -3;
	}
	res=mysql_query_starts(MYSQL_LOCAL_INDEX,&con);

	res1=mysql_query_starts(MYSQL_REMOTE_INDEX,&rcon);
	for(param_update_type * param = *plist;param; param = param->next)
	{
		num=snprintf(sql,sql_size,"update hy_wnet_param_values set param_value=%0.4f,update_time='%s' where param_id=%d;",param->param_value,param->update_time,param->param_id);

		if(mysql_real_query(&con->conn,sql,num) == 0)
		{
			mysql_free_result(mysql_store_result(&con->conn));
		}
		else
		{
			ERR_LOG("update hy_wnet_param_values error [%d] %s\n[%s]", mysql_errno(&con->conn),mysql_error(&con->conn),sql);
			break;
		}
		if(rcon)
		{
			if(mysql_real_query(&rcon->conn,sql,num) == 0)
			{
				mysql_free_result(mysql_store_result(&rcon->conn));
			}
			else
			{
				ERR_LOG("insert hy_wnet_param_historys error [%d] %s\n[%s]", mysql_errno(&rcon->conn),mysql_error(&rcon->conn),sql);
				//break;
			}

			num=snprintf(sql,sql_size,"insert into hy_wnet_param_historys values(0,%d,%0.4f,'%s');",param->param_id,param->param_value,param->update_time);
			if(mysql_real_query(&rcon->conn,sql,num) == 0)
			{
				mysql_free_result(mysql_store_result(&rcon->conn));
			}
			else
			{
				ERR_LOG("insert hy_wnet_param_historys error [%d] %s\n[%s]", mysql_errno(&rcon->conn),mysql_error(&rcon->conn),sql);
				//break;
			}

		}
	}
	free(sql);
	con->err_flag=res;
	res=mysql_query_ends(MYSQL_LOCAL_INDEX,con,res);
	if(rcon)
	{
		rcon->err_flag=res1;
		if(mysql_query_ends(MYSQL_REMOTE_INDEX,rcon,res1)!=0)
		{
			ERR_LOG("mysql commit error [%d] %s", mysql_errno(&rcon->conn),mysql_error(&rcon->conn));
		}
	}else
	{
		ERR_LOG("can not get remote connections");
	}
	return res;
}

int get_measures_info(measure_type_define**list)
{
	int flag=0;
	MYSQL_RES *res_ptr;
	MYSQL_ROW row;
	const char *sql = "select mr_id,mr_type,mr_addr,mr_value_format from hy_wnet_measurers where mr_addr is not null;";
	res_ptr=mysql_execute_querys(MYSQL_LOCAL_INDEX,sql,strlen(sql),&flag);
	if(res_ptr!=NULL)
	{
		if(flag==0)
		{
			my_ulonglong rownums = mysql_num_rows(res_ptr);
			for(int i=0;i<rownums;i++)
			{
				row = mysql_fetch_row(res_ptr);
				measure_type_define * mr = malloc(sizeof(measure_type_define));
				if(mr)
				{
					mr->prev=NULL;
					mr->next=NULL;
					mr->mr_id = IntValue(row[0]);
					mr->mr_type = IntValue(row[1]);
					strncpy(mr->mr_addr,row[2],sizeof(mr->mr_addr));
					if(row[3]!=NULL)
					{
						strncpy(mr->mr_value_format,row[3],sizeof(mr->mr_value_format));
					}
					LIST_PUSH(list,mr,measure_type_define);
				}
			}
		}
		mysql_free_result(res_ptr);	//释放结果集占用内存
	}
	return flag;
}

int get_measures_one(int mr_id,measure_type_define*mr)
{
	int flag=0;
	MYSQL_RES *res_ptr;
	MYSQL_ROW row;
	char sql[256];
	int num = snprintf(sql,sizeof(sql),"select mr_id,mr_type,mr_addr,mr_value_format from hy_wnet_measurers where mr_id=%d;",mr_id);

	res_ptr=mysql_execute_querys(MYSQL_LOCAL_INDEX,sql,num,&flag);
	if(res_ptr!=NULL)
	{
		if(flag==0)
		{
			my_ulonglong rownums = mysql_num_rows(res_ptr);
			if(rownums>0)
			{
				row = mysql_fetch_row(res_ptr);
				mr->prev=NULL;
				mr->next=NULL;
				mr->mr_id = IntValue(row[0]);
				mr->mr_type = IntValue(row[1]);
				strncpy(mr->mr_addr,row[2],sizeof(mr->mr_addr));
				if(row[3]!=NULL)
				{
					strncpy(mr->mr_value_format,row[3],sizeof(mr->mr_value_format));
				}
			}else
			{
				mr->mr_id=0;
			}
		}
		mysql_free_result(res_ptr);	//释放结果集占用内存
	}
	return flag;
}

int updae_measures_value(measure_type_define**list)
{
	int res,res1;
	char *sql;
	size_t sql_size = 1024;
	size_t num;
	mysql_conn*con=NULL;
	mysql_conn*rcon=NULL;
	if(*list == NULL)
	{
		return 0;
	}
	sql = malloc(sql_size);
	if(sql == NULL)
	{
		return -3;
	}
	res=mysql_query_starts(MYSQL_LOCAL_INDEX,&con);

	res1=mysql_query_starts(MYSQL_REMOTE_INDEX,&rcon);
	for(measure_type_define * mr = *list;mr; mr = mr->next)
	{
		num=snprintf(sql,sql_size,"update hy_wnet_measurer_values set mr_value='%s',mr_state=%d,update_time=now() where mr_id=%d;",mr->mr_value_format,mr->mr_state,mr->mr_id);
		if(mysql_real_query(&con->conn,sql,num) == 0)
		{
			mysql_free_result(mysql_store_result(&con->conn));
		}
		else
		{
			ERR_LOG("update local hy_wnet_measure_values error [%d] %s\n[%s]", mysql_errno(&con->conn),mysql_error(&con->conn),sql);
			//break;
		}
		if(rcon)
		{
			if(mysql_real_query(&rcon->conn,sql,num) == 0)
			{
				mysql_free_result(mysql_store_result(&rcon->conn));
			}
			else
			{
				ERR_LOG("update remote hy_wnet_measurer_values error [%d] %s\n[%s]", mysql_errno(&rcon->conn),mysql_error(&rcon->conn),sql);
				//break;
			}
		}

		num=snprintf(sql,sql_size,"insert into hy_wnet_measurer_historys values(0,%d,'%s',%d,now());",mr->mr_id,mr->mr_value_format,mr->mr_state);

		if(mysql_real_query(&con->conn,sql,num) == 0)
		{
			mysql_free_result(mysql_store_result(&con->conn));
		}
		else
		{
			ERR_LOG("insert remote hy_wnet_measurer_historys error [%d] %s\n[%s]", mysql_errno(&con->conn),mysql_error(&con->conn),sql);
			//break;
		}
	}
	free(sql);
	con->err_flag=res;
	res=mysql_query_ends(MYSQL_LOCAL_INDEX,con,res);
	if(rcon)
	{
		rcon->err_flag=res1;
		if(mysql_query_ends(MYSQL_REMOTE_INDEX,rcon,res1)!=0)
		{
			ERR_LOG("mysql commit error [%d] %s", mysql_errno(&rcon->conn),mysql_error(&rcon->conn));
		}
	}
	return res;
}

int wnet_tab_rollback(const char*tab,int max_count,int roll_times)
{
	int count=0;
	char sql[1024];
	int res=mysql_get_tab_rows(MYSQL_LOCAL_INDEX,tab);
	if(res<0)
	{
		ERR_LOG("get hy_wnet_arm301_historys count error");
		return res;
	}
	count=res;
	if(count>max_count||max_count==0)
	{
		int num;
		num=snprintf(sql,sizeof(sql),"drop table if exists %s%d;",tab,roll_times);
		for(int i=roll_times-1;i>0;i--)
		{
			num+=snprintf(sql+num,sizeof(sql)-num,"create table if not exists %s%d like %s;",tab,i,tab);
			num+=snprintf(sql+num,sizeof(sql)-num,"rename table %s%d to %s%d;",tab,i,tab,i+1);
		}
		num+=snprintf(sql+num,sizeof(sql)-num,"create table if not exists %s_tmp like %s;",tab,tab);
		num+=snprintf(sql+num,sizeof(sql)-num,"rename table %s to %s1;",tab,tab);
		num+=snprintf(sql+num,sizeof(sql)-num,"rename table %s_tmp to %s;",tab,tab);

		//sql="create table if not exists hy_wnet_arm301_historys1 like hy_wnet_arm301_historys;"
		//		"truncate table hy_wnet_arm301_historys1;"
		//		"insert hy_wnet_arm301_historys1 select * from hy_wnet_arm301_historys;"
		//		"truncate table hy_wnet_arm301_historys;";
		//sql="rename table hy_wnet_param_historys_tmp to hy_wnet_param_historys;";

		res= mysql_none_querys(MYSQL_LOCAL_INDEX,sql,num);
		if(res!=0)
		{
			ERR_LOG("%s roll back error sql(%s)",tab,sql);
		}
		return res;
	}else
	{
		return 0;
	}
}


#endif


void mysql_local_init()
{
	const char *host="localhost";
	const char *username="wnet";
	const char *passwd="wnet";
	const char *database="wnet";
	int port=3306;
	int max_keep_connectios=DATABASE0_KEPP_CONNECTIONS;
	mysql_pool_inits(MYSQL_LOCAL_INDEX,host,username,passwd,database,port,max_keep_connectios);
	if(get_mysql_connections_nums(MYSQL_LOCAL_INDEX)<=0)
	{
		ERR_LOG("No local connections was created");
		exit(-1);
	}else
	{
		RUN_LOG("loacl mysql connections pool was created");
	}
}

void mysql_remote_init()
{
	const char *host="192.168.1.131";
	const char *username="wnet";
	const char *passwd="wnet";
	const char *database="wnet";
	int port=3306;
	int max_keep_connectios=DATABASE1_KEPP_CONNECTIONS;
	mysql_pool_inits(MYSQL_REMOTE_INDEX,host,username,passwd,database,port,max_keep_connectios);
	if(get_mysql_connections_nums(MYSQL_REMOTE_INDEX)<=0)
	{
		ERR_LOG("No remote connections was created");
	}else
	{
		RUN_LOG("remote mysql connections pool was created");
	}
}

void mysql_local_uninit()
{
	destory_mysql_pools(MYSQL_LOCAL_INDEX);
	RUN_LOG("loacl mysql connections pool was destoryed");
}

void mysql_remote_uninit()
{
	destory_mysql_pools(MYSQL_REMOTE_INDEX);
	RUN_LOG("remote mysql connections pool was destoryed");
}

