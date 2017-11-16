// 
// 创建人： levy
// 创建时间：Mar 1, 2017
// 功能：sqlite_assist.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Mar 1, 2017                  levy          初版
// 

#include <bms_def.h>
#include <conf.h>
#include "sqlite_assist.h"


//static sqlite3* BMS_Db;
static char bms_database[32]={0};
static Boolean is_db_init=FALSE;

/*
 * sqlite3 数据库文件 路经信息读取
 * */
int bms_db_init()
{
	int res=0;
	char * confilename = getenv("BMS_CONFIG_FILE");
	if (confilename == NULL)
	{
		confilename="/opt/bms/etc/bms.cnf";

	}
	if (access(confilename, F_OK) == -1)
	{
		ERR_LOG("BMS","配置文件不存在:%s",confilename);
		return -1;
	}
	res=GetProfileString(confilename,"sqlite3","database",bms_database);
	if(res==0)
		return 0;
	else return -1;
}
/*
 * 打开数据库bms_database,ppdb返回打开的数据库指针
 * @return 0 sucess
 * @return -1 open error
 * */
int bms_sql_open(sqlite3** ppdb)
{
	if(is_db_init==FALSE)
	{
		if(bms_db_init()==0)
		{
			is_db_init=TRUE;
		}else
		{
			ERR_LOG("BMS","bms 数据库初始化失败");
		}
	}

	if(access(bms_database, F_OK) == -1)
	{
		ERR_LOG("BMS","数据库文件%s不存在",bms_database);
		return -1;
	}
	int res=0;
	//sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
	res=sqlite3_open(bms_database,ppdb);
	if(res!=SQLITE_OK)
	{
		ERR_LOG("BMS","数据库打开失败");
		return -1;
	}
	return 0;
}
//关闭数据库
inline void bms_close(sqlite3 * db)
{
	sqlite3_close(db);
}

/*
 *执行数据库更新插入语句，无返回结果
 * */
int bms_sql_exec(const char * sql)
{
	sqlite3 * bms_db;
	int res=bms_sql_open(&bms_db);
	if(res!=0)
	{
		return -1;
	}
	res=sqlite3_exec(bms_db,sql,NULL,NULL,NULL);
	if (res != SQLITE_OK)
	{
		const char * err_msg=sqlite3_errmsg(bms_db);
		int err_code=sqlite3_errcode(bms_db);
		ERR_LOG("BMS","数据库执行错误sql（%s）：%d,%s",sql,err_code,err_msg);
		sqlite3_free((void*)err_msg);
		bms_close(bms_db);
		return -1;
	}
	bms_close(bms_db);
	return 0;
}
/*
 * 执行数据库查询语句
 * */
int bms_sql_querry(const char *sql,char ***PResult,int *pnRow,int *pnColumn)
{
	sqlite3 * bms_db;
	int res=bms_sql_open(&bms_db);
	if(res!=0)
	{
		return -1;
	}
	res=sqlite3_get_table(bms_db,sql,PResult,pnRow,pnColumn,NULL);
	if (res != SQLITE_OK)
	{
		const char * err_msg=sqlite3_errmsg(bms_db);
		int err_code=sqlite3_errcode(bms_db);
		ERR_LOG("BMS","数据库查询错误sql（%s）：%d,%s",sql,err_code,err_msg);
		sqlite3_free((void*)err_msg);
		bms_close(bms_db);
		return -1;
	}
	bms_close(bms_db);
	return 0;
}

int bms_getjsonobject(cJSON **json,int group_id )
{
	char sql[MAX_SQL_STRING_LENGTH];
	char **result;
	int nRow,nColumn;
	int res=0;
	snprintf(sql,MAX_SQL_STRING_LENGTH,"select param_object from hy_bms_jsonparam where group_id=%d;",group_id);
	res=bms_sql_querry(sql,&result,&nRow,&nColumn);
	if(res!=0)
	{
		ERR_LOG("BMS","get hy_bms_jsonparam.param_object error.");
		sqlite3_free_table(result);
		return -1;
	}else if(nRow ==0)
	{
		ERR_LOG("BMS","hy_bms_jsonparam has no groupid %d",group_id);
		sqlite3_free_table(result);
		return -1;
	}
	*json=cJSON_Parse(result[1]);
	sqlite3_free_table(result);
	return USR_SUCESS;
}
int bms_updatejsonobject(cJSON *json,int group_id)
{
	char sql[MAX_SQL_STRING_LENGTH];
	char *jstr=cJSON_PrintBuffered(json,MAX_JSON_STRING_LENGTH,0);
	snprintf(sql,MAX_SQL_STRING_LENGTH,"update hy_bms_jsonparam set param_object='%s',update_time=datetime(CURRENT_TIMESTAMP,'localtime') where group_id=%d;",jstr,group_id);
	free(jstr);
	int res=bms_sql_exec(sql);
	if(res!=0)
	{
		ERR_LOG("BMS","hy_bms_jsonparam update error.");
		return -1;
	}
	return USR_SUCESS;
}

int bms_insertjsonobject(cJSON *json,int group_id)
{
	char sql[MAX_SQL_STRING_LENGTH];
	char *jstr=cJSON_PrintBuffered(json,MAX_JSON_STRING_LENGTH,0);
	snprintf(sql,MAX_SQL_STRING_LENGTH,"replace into hy_bms_jsonparam values(%d,'%s',datetime(CURRENT_TIMESTAMP,'localtime'));",group_id,jstr);
	free(jstr);
	int res=bms_sql_exec(sql);
	if(res!=0)
	{
		ERR_LOG("BMS","hy_bms_jsonparam replace error.");
		return -1;
	}
	return USR_SUCESS;
}


//void bms_get_data()
//{
//	char *err_mesg;
//	//sqlite3_exec(BMS_Db,"",NULL,NULL,&err_mesg);
//	char *sql;
//	char **result;
//	int nRow,nColum;
//	int res=0;
//	res=sqlite3_get_table(BMS_Db,sql,&result,&nRow,&nColum,&err_mesg);
//	if (res != SQLITE_OK)
//	{
//		//sqlite3_close(db);
//		//sqlite3_errmsg();
//		//sqlite3_errcode();
//		//sqlite3_free(err_mesg);
//		return ;
//	}
//	for (int i=1;i<nRow;i++)
//	{
//		for(int j=0;j<nColum;j++)
//		{
//			result[i*nColum+nColum];
//		}
//	}
//	sqlite3_free_table(result);
//}


