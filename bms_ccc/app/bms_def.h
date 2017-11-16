// 
// 创建人： levy
// 创建时间：Mar 1, 2017
// 功能：bms_def.h
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Mar 1, 2017                  levy          初版
// 

#ifndef BMS_DEF_H_
#define BMS_DEF_H_

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include "cJSON.h"

#define CELL_BATTERY_NUM 16
#define MAIN_BATTERY_GROUP_ID 0xff

#define JSON_INT_VALUE_SET(set,json,str) do{\
	cJSON * result=cJSON_GetObjectItem(json,str);\
	if(result!=NULL)\
		set=result->valueint;\
	else ERR_LOG("BMS","JSON object has no key (%s)",str);\
}while(0)

#define JSON_DOUBLE_VALUE_SET(set,json,str) do{\
	cJSON * result=cJSON_GetObjectItem(json,str);\
	if(result!=NULL)\
		set=result->valuedouble;\
	else ERR_LOG("BMS","JSON object has no key (%s)",str);\
}while(0)

#define JSON_STR_VALUE_SET(set,json,str) do{\
	cJSON * result=cJSON_GetObjectItem(json,str);\
	if(result!=NULL)\
		strcpy(set,result->valuestring);\
	else ERR_LOG("BMS","JSON object has no key (%s)",str);\
}while(0)


typedef enum
{
	FALSE = 0, TRUE = 1
} Boolean;

typedef enum
{
	USR_SUCESS = 0, SQL_QURY_ERR,			//执行SQL语句出错误
	USR_TAB_ERR,			//读取用户数据表出错
	USR_TAB_NULL,
	USR_SQL_UNCONECT,		//用户数据未连接
	USR_CONF_FAIL,			//VDGS配置文件读取失败
	USR_MYSQL_FAIL,			//mysq配置错误
	USR_PARAM_NULL,			//参数为空错误
	USR_MEM_ERR,			//内存分配错误
	USR_THREAD_ERR,			//线程错误
	USR_SQL_CONNECT_ERR,	//数据库链接错误
	USR_DATABASE_ERR,
	USR_COM_ERR,
	USR_MSSQL_ERR,
	USR_MYSQL_ERR
} SQL_Usr_Err;

typedef struct
{
	int id;  		//蓄电池单元
	double volatge;	//电池单元电压
	double current;	//电池单元电流
	double temp;		//温度
	double res_elc;	//剩余电量
	double total;		//总容量
	int relay;			//效率

}Battery_main_def;

typedef struct
{
	int id;  		//蓄电池单元
	double volatge;	//电池单元电压
	double current;	//电池单元电流
	double temp;		//温度
	double res_elc;	//剩余电量
	double total;		//总容量
	double u;			//效率

}Battery_cell_def;



#endif /* BMS_DEF_H_ */
