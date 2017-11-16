// 
// 创建人： levy
// 创建时间：Jun 5, 2017
// 功能：mysql_assist.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Jun 5, 2017                  levy          初版
// 

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "mysql_assist.h"

/*从tab中的获取猎命为column的值value，只可以用来查询单行表
 * @value 包括四种类型(int,double,char*)
 * @return 查询结果
 * */
int mysql_get_tab_value(const char * tab ,const char * column,void *value,size_t size)
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
	res_ptr=mysql_execute_query(sql,num,&flag);
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
int mysql_update_tab_value(const char *tab,const char *column,int type,const char * val, ...)
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
	res=mysql_none_query(sql,num);
	free(va_buf);
	free(sql);
	return res;
}

size_t make_update_tag_string(char *sql,size_t size,TagStruct*tag)
{
	size_t len=0;
	char val_buf[32];
	if(tag->error == 0)
	{
		cip_value2string(val_buf,&tag->value,tag->type);
		len=snprintf(sql,size,"update hy_pbbms_tagsvalue set tagtype=%d,tagvalue='%s',update_time=now(),error_state=0 where tagname='%s';",
				tag->type,val_buf,tag->tag);
	}else
	{
		len=snprintf(sql,size,"update hy_pbbms_tagsvalue set update_time=now(),error_state=%d where tagname='%s';",
				tag->error,tag->tag);
	}
	return len;
}

size_t make_histroy_tag_string(char *sql,size_t size,TagStruct*tag)
{
	size_t len=0;
	char val_buf[32];
	if(tag->error ==0)
	{
		cip_value2string(val_buf,&tag->value,tag->type);
		len = snprintf(sql,size,"insert into hy_pbbms_tagshistory (id_num,tagname,tagtype,tagvalue,update_time,error_state) values(0,'%s',%d,'%s',now(),0);",
				tag->tag,tag->type,val_buf);
	}
	return len;
}

int cip_update_tag_value(TagStruct*taglist)
{
	int res;
	char *sql;
	size_t sql_size = 256;
	size_t num;
	mysql_conn*con;
	if(taglist == NULL)
	{
		return 0;
	}
	sql = malloc(sql_size);
	if(sql == NULL)
	{
		return -3;
	}
	res=mysql_query_start(&con);
	if(res<0)
	{
		return res;
	}
	for(TagStruct * tag = taglist;tag; tag = tag->next)
	{
		num = make_update_tag_string(sql,sql_size,tag);
		res=mysql_real_query(&con->conn,sql,num);
		if(res == 0)
		{
			mysql_free_result(mysql_store_result(&con->conn));
		}
		else
		{
			DEBUG_PRINTF("update tag value sql error [%d] %s\n", mysql_errno(&con->conn),mysql_error(&con->conn));
			break;
		}
		num = make_histroy_tag_string(sql,sql_size,tag);
		if(num>0)
		res=mysql_real_query(&con->conn,sql,num);
		if(res == 0)
		{
			mysql_free_result(mysql_store_result(&con->conn));
		}
		else
		{
			DEBUG_PRINTF(" insert history tag sql error [%d] %s\n", mysql_errno(&con->conn),mysql_error(&con->conn));
			break;
		}
	}
	free(sql);
	con->err_flag=res;
	res=mysql_query_end(con,res);
	return res;
}

int cip_get_tag_list(TagStruct**taglist)
{
	int flag=0;
	MYSQL_RES *res_ptr;
	TagStruct * tag;
	MYSQL_ROW row;
	const char *sql="select tagname from hy_pbbms_tagsvalue where is_disabled=0;";
	res_ptr=mysql_execute_query(sql,strlen(sql),&flag);
	if(res_ptr!=NULL)
	{
		my_ulonglong rownums = mysql_num_rows(res_ptr);
		for(int i=0;i<rownums;i++)
		{
			row = mysql_fetch_row(res_ptr);
			tag=CreatTagStruct(row[0],0);
			if(tag)
			{
				TagStructPush(taglist,tag);
			}
		}
		mysql_free_result(res_ptr);	//释放结果集占用内存
	}
	return flag;
}

int cip_get_write_taglist(RecordTagStruct**list)
{
	int flag=0;
	MYSQL_RES *res_ptr;
	RecordTagStruct *rtag;
	MYSQL_ROW row;
	const char *sql="select tagname,tagtype,tagvalue,update_time from hy_pbbms_write_tagvalue;";
	res_ptr=mysql_execute_query(sql,strlen(sql),&flag);
	if(res_ptr!=NULL)
	{
		my_ulonglong rownums = mysql_num_rows(res_ptr);
		for(int i=0;i<rownums;i++)
		{
			if(row[0] == NULL)continue;
			row = mysql_fetch_row(res_ptr);
			rtag = malloc(sizeof(RecordTagStruct));
			if(rtag == NULL)
			{
				flag= -3;
				break;
			}
			strncpy(rtag->tagname,row[0],sizeof(rtag->tagname));
			strncpy(rtag->tag.tag,rtag->tagname,sizeof(rtag->tagname));
			rtag->tag.type = IntValue(row[1]);
			rtag->tag.rw_flag =1;
			rtag->tag.error = 0;
			rtag->mtag=NULL;
			cip_get_value_from_string(&rtag->tag.value,row[2],rtag->tag.type);
			if(row[3])
			{
				strncpy(rtag->update_time,row[3],sizeof(rtag->update_time));
			}
			else
			{
				rtag->update_time[0]=0;
			}
			SignleList_Push(list,rtag,RecordTagStruct);
		}
		if(rownums>0)
		{
			mysql_none_query("delete from hy_pbbms_write_tagvalue;",strlen("delete from hy_pbbms_write_tagvalue;"));
		}
		mysql_free_result(res_ptr);	//释放结果集占用内存
	}

	return flag;
}

int cip_update_write_history(RecordTagStruct * list)
{
	int res;
	char *sql;
	size_t sql_size = 256;
	size_t num;
	mysql_conn*con;
	if(list == NULL)
	{
		return 0;
	}
	sql = malloc(sql_size);
	if(sql == NULL)
	{
		return -3;
	}
	res=mysql_query_start(&con);
	if(res<0)
	{
		return res;
	}
	for(RecordTagStruct * tag = list;tag; tag = tag->next)
	{

		char val_buf[32];
		cip_value2string(val_buf,&tag->tag.value,tag->tag.type);
		num=snprintf(sql,sql_size,"insert into hy_pbbms_write_tagvalue_history(id_num,tagname,tagtype,tagvalue,error_state,update_time,record_time) "
				"values(0,'%s',%d,'%s',%d,'%s',now());",tag->tagname,tag->tag.type,val_buf,tag->tag.error,tag->update_time);
		res=mysql_real_query(&con->conn,sql,num);
		if(res == 0)
		{
			mysql_free_result(mysql_store_result(&con->conn));
		}
		else
		{
			break;
		}

	}
	free(sql);
	con->err_flag=res;
	res=mysql_query_end(con,res);
	return res;
}
