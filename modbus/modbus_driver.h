// 
// 创建人： wnet
// 创建时间：Aug 24, 2017
// 功能：modbus_driver.h
// Copyright (c) 2016 wnet. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Aug 24, 2017                  wnet          初版
// 


#ifndef MODBUS_DRIVER_H_
#define MODBUS_DRIVER_H_
#include <stdint.h>

#define MODBUS_VTIME 1
#define MODBUS_VMIN 256
#define MODBUS_MSG_MIN_LEN 4

#define MODBUS_OK											0x00
#define MODBUS_CMD_ERR										0x01
#define MODBUS_ADDR_ERR										0x02
#define MODBUS_DATA_ERR										0x03
#define MODBUS_TIME_OVER									0x04
#define MODBUS_DEVICE_ERR									0x05
#define MODBUS_CRC_ERR										0x08
#define MODBUS_BUSY_ERR										0x06


typedef struct
{
	uint8_t addr;
	uint8_t cmd;
	uint8_t err_state;
	uint8_t res;
	uint16_t reg_addr;
	uint16_t reg_num;
	uint16_t reg_value[256];

}modbus_object;

uint16_t crc16_check(uint8_t *pDataIn, uint16_t iLenIn);
int modebus_cmd(int com_id,modbus_object * obj);
int modbus_com_open(const char * com_name,int bandrate);
void modbus_com_close(int com_id);


#endif /* MODBUS_DRIVER_H_ */
