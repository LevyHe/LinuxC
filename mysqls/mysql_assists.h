// 
// 创建人： levy
// 创建时间：Jun 6, 2017
// 功能：mysql_assist.h
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Jun 6, 2017                  levy          初版
// 

#if 1
#ifndef MYSQL_ASSISTS_H_
#define MYSQL_ASSISTS_H_
#include "main.h"
#include "can_pro.h"
#include "mysql_pools.h"
#define MYSQL_LOCAL_INDEX 0
#define MYSQL_REMOTE_INDEX 1


typedef struct param_define
{
	int param_id;
	int parts_id;
	int node_id;
	char param_io[8];
	double coefficient;
	double offset_value;
	double threshold_value;
	double param_value;
	int param_type;

}param_define;


typedef struct param_update_type
{
	struct param_update_type *next;
	struct param_update_type *prev;
	int param_id;
	double param_value;
	char update_time[32];
}param_update_type;


typedef struct measure_type_define
{
	struct measure_type_define *next;
	struct measure_type_define *prev;
	int mr_id;
	int mr_type;
	int mr_state;
	char mr_addr[32];
	char mr_value_format[256];

}measure_type_define;


int wnet_get_arm_node_list(wnet_arm301_define ** list);
int wnet_update_arm_node_list(wnet_arm301_define**list);
int update_read_param_value(param_update_type** plist);
int get_param_information(int paramid,param_define *param);
int get_param_information_all(int *p_length,param_define **param_ptr);
int mysql_get_tab_value(uint32_t index,const char * tab ,const char * column,void *value,size_t size);
int mysql_update_tab_value(uint32_t index,const char *tab,const char *column,int type,const char * val, ...);
int update_arm301_value(int node_id,int value,char* name);
int update_set_param_value(int param_id,char*param_value);
int get_measures_info(measure_type_define**list);
int get_measures_one(int mr_id,measure_type_define*mr);
int updae_measures_value(measure_type_define**list);
int wnet_tab_rollback(const char*tab,int max_count,int roll_times);
void mysql_local_init();
void mysql_remote_init();
void mysql_local_uninit();
void mysql_remote_uninit();
//int cip_get_tag_list(TagStruct**taglist);
////int cip_history_tag_value(TagStruct*taglist);
//int cip_update_tag_value(TagStruct*taglist);
//int cip_get_write_taglist(RecordTagStruct**list);
//int cip_update_write_history(RecordTagStruct * list);
#endif /* MYSQL_ASSISTS_H_ */
#endif
