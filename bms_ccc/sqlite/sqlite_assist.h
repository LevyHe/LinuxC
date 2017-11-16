// 
// 创建人： levy
// 创建时间：Mar 1, 2017
// 功能：sqlite_assist.h
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Mar 1, 2017                  levy          初版
// 


#ifndef SQLITE_ASSIST_H_
#define SQLITE_ASSIST_H_
#include <sqlite3.h>
#include "cJSON.h"
#include "mylog.h"

#define MAX_SQL_STRING_LENGTH 4096
#define MAX_JSON_STRING_LENGTH 2048

//#include <sqlite3ext.h>
int bms_db_init();
int bms_sql_open(sqlite3** ppdb);
void bms_close(sqlite3 * db);
int bms_sql_exec(const char * sql);
int bms_sql_querry(const char *sql,char ***PResult,int *pnRow,int *pnColumn);
int bms_getjsonobject(cJSON **json,int group_id );
int bms_updatejsonobject(cJSON *json,int group_id);


#endif /* SQLITE_ASSIST_H_ */
