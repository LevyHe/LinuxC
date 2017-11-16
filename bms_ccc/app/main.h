// 
// 创建人： levy
// 创建时间：Mar 24, 2017
// 功能：main.h
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Mar 24, 2017                  levy          初版
// 


#ifndef MAIN_H_
#define MAIN_H_

void bms_init();
cJSON * convert_battery_main_to_json(Battery_main_def *bms);
cJSON *convert_battery_cell_to_json(Battery_cell_def*cell);
void update_battery_main_from_json(Battery_main_def*bms,cJSON*json);
void update_battery_cell_from_json(Battery_cell_def*cell,cJSON*json);
Battery_main_def * get_battery_main_param();

#endif /* MAIN_H_ */
