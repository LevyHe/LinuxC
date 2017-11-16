// 
// 创建人： levy
// 创建时间：Mar 23, 2017
// 功能：battery_sample.h
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Mar 23, 2017                  levy          初版
// 


#ifndef BATTERY_SAMPLE_H_
#define BATTERY_SAMPLE_H_

#define BATTERY_SAMPLE_VTIME 1
#define BATTERY_SAMPLE_VMIN 512
#define BATTERY_SAMPLE_MSG_MIN_LEN 7

typedef struct
{
	uint8_t header1;
	uint8_t header2;
	uint8_t cmd_code;//功能码
	uint16_t msg_len;//数据包消息长度
	uint8_t uint_num;//参数组数目
	uint8_t *uint_array[256];
	uint16_t crc;
}sample_package_def;

#endif /* BATTERY_SAMPLE_H_ */
