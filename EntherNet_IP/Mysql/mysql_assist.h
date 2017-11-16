// 
// 创建人： levy
// 创建时间：Jun 6, 2017
// 功能：mysql_assist.h
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Jun 6, 2017                  levy          初版
// 


#ifndef MYSQL_ASSIST_H_
#define MYSQL_ASSIST_H_
#include "EntherNet.h"
#include "mysql_pool.h"
#include "pbbms.h"

int mysql_get_tab_value(const char * tab ,const char * column,void *value,size_t size);
int mysql_update_tab_value(const char *tab,const char *column,int type,const char * val, ...);
int cip_get_tag_list(TagStruct**taglist);
//int cip_history_tag_value(TagStruct*taglist);
int cip_update_tag_value(TagStruct*taglist);
int cip_get_write_taglist(RecordTagStruct**list);
int cip_update_write_history(RecordTagStruct * list);
#endif /* MYSQL_ASSIST_H_ */
