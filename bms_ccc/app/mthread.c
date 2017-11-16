// 
// 创建人： levy
// 创建时间：Mar 4, 2017
// 功能：mthread.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Mar 4, 2017                  levy          初版
// 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bms_def.h>
#include "sqlite_assist.h"

static Battery_cell_def cell_array[256];
static Battery_main_def battery;

cJSON * convert_battery_main_to_json(Battery_main_def *bms)
{
	cJSON * root=cJSON_CreateObject();
	cJSON_AddNumberToObject(root,"id",bms->id);
	cJSON_AddNumberToObject(root,"volatge",bms->volatge);
	cJSON_AddNumberToObject(root,"current",bms->current);
	cJSON_AddNumberToObject(root,"temp",bms->temp);
	cJSON_AddNumberToObject(root,"res_elc",bms->res_elc);
	cJSON_AddNumberToObject(root,"total",bms->total);
	cJSON_AddNumberToObject(root,"relay",bms->relay);
	return root;
}

void update_battery_main_from_json(Battery_main_def*bms,cJSON*json)
{
	if(json==NULL||bms==NULL)
		return;
	//battery.id=cJSON_GetObjectItem(json,"id")->valueint;
	JSON_INT_VALUE_SET(bms->id,json,"id");
	JSON_DOUBLE_VALUE_SET(bms->volatge,json,"volatge");
	JSON_DOUBLE_VALUE_SET(bms->current,json,"current");
	JSON_DOUBLE_VALUE_SET(bms->temp,json,"temp");
	JSON_DOUBLE_VALUE_SET(bms->res_elc,json,"res_elc");
	JSON_DOUBLE_VALUE_SET(bms->total,json,"total");
	JSON_INT_VALUE_SET(bms->relay,json,"relay");
}

void update_battery_cell_from_json(Battery_cell_def*cell,cJSON*json)
{
	if(json==NULL||cell==NULL)
		return;
	//battery.id=cJSON_GetObjectItem(json,"id")->valueint;
	JSON_INT_VALUE_SET(cell->id,json,"id");
	JSON_DOUBLE_VALUE_SET(cell->volatge,json,"volatge");
	JSON_DOUBLE_VALUE_SET(cell->current,json,"current");
	JSON_DOUBLE_VALUE_SET(cell->temp,json,"temp");
	JSON_DOUBLE_VALUE_SET(cell->res_elc,json,"res_elc");
	JSON_DOUBLE_VALUE_SET(cell->total,json,"total");
	JSON_DOUBLE_VALUE_SET(cell->u,json,"u");
}

cJSON *convert_battery_cell_to_json(Battery_cell_def*cell)
{
	cJSON * root=cJSON_CreateObject();
	cJSON_AddNumberToObject(root,"id",cell->id);
	cJSON_AddNumberToObject(root,"volatge",cell->volatge);
	cJSON_AddNumberToObject(root,"current",cell->current);
	cJSON_AddNumberToObject(root,"temp",cell->temp);
	cJSON_AddNumberToObject(root,"res_elc",cell->res_elc);
	cJSON_AddNumberToObject(root,"total",cell->total);
	cJSON_AddNumberToObject(root,"u",cell->u);

	return root;
}

cJSON * getJSON_from_string(const char *value)
{
	return cJSON_Parse(value);
}

void bms_init()
{
	//初始化cell_array 数组
	for(int i=0;i<CELL_BATTERY_NUM;i++)
	{
		cell_array[i].id=i;
	}
	//读取数据库中已有的参数表

	for(int i=0;i<CELL_BATTERY_NUM;i++)
	{
		cJSON* json;
		if(bms_getjsonobject(&json,cell_array[i].id)==0)
		{
			update_battery_cell_from_json(&(cell_array[i]),json);
		}else
		{
			json=convert_battery_cell_to_json(&(cell_array[i]));
			bms_insertjsonobject(json,cell_array[i].id);
		}
		cJSON_Delete(json);
	}
	cJSON* main_json;
	battery.id=MAIN_BATTERY_GROUP_ID;
	if(bms_getjsonobject(&main_json,MAIN_BATTERY_GROUP_ID)==0)
	{
		update_battery_main_from_json(&battery,main_json);
	}else
	{
		main_json=convert_battery_main_to_json(&battery);
		bms_insertjsonobject(main_json,MAIN_BATTERY_GROUP_ID);
	}
	cJSON_Delete(main_json);
	//反向更新数据表
}

Battery_main_def * get_battery_main_param()
{
	return &battery;
}

