// 
// 创建人： levy
// 创建时间：Mar 1, 2017
// 功能：DwinDriver.h
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Mar 1, 2017                  levy          初版
// 


#ifndef DWINDRIVER_H_
#define DWINDRIVER_H_

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>


#define KEY_CODE 0x4F
#define PIC_ID	0x03

#define DWIN_VTIME 1
#define DWIN_VMIN 128
#define DWIN_MinMsgLen 4


typedef struct
{
	int timeoutcont;
	uint8_t Ver;
	uint8_t LED_SET;
	uint8_t BZ_TIME;
	uint16_t PCI_ID;
	uint8_t TP_FLAG;
	uint8_t TP_Status;
	uint16_t TP_Postion[2];
	uint8_t TP_En;
	uint32_t RUN_TIME;

} DwinRegDef_P;	//部分DGUS寄存器定义


typedef struct
{
	uint8_t header1;
	uint8_t header2;
	uint8_t len;
	uint8_t cmd;
	uint8_t buf[256];
	uint8_t crch;
	uint8_t crcl;

} DWINCmdDef;

typedef struct
{
	uint16_t saddr;
	uint8_t vnum;
	uint16_t vbuf[128];

} DWINVarDef;

typedef struct
{
	uint8_t sreg;
	uint8_t num;
	uint8_t rbuf[256];
} DWINRegDef;
void *ptTsProcess(void *arp);
extern void *ptVarProcess(void *arp);
extern void *ptRegProcess(void *app);
inline void lockserial();
int com_start();
void com_close(void);
int GetDWINCmd(DWINCmdDef*cmd, uint8_t *buf, int num);
int GetDWINVar(DWINVarDef*var, uint8_t * buf, int num);
int GetDWINReg(DWINRegDef *reg, uint8_t *buf, int num);
void DWINWriteReg(uint8_t reg, uint8_t value);
void DWINWriteRegs(uint8_t reg, uint8_t num, const uint8_t *buf);
void DWIMReadRegs(uint8_t addr, uint8_t num);
void DWINWriteVariable(uint16_t adr, const uint16_t value);
void DWINWriteVariables(uint16_t adr, uint8_t num, uint16_t *buf);
void DWINWriteVararry(uint16_t adr, uint8_t num, uint8_t*buf);
void DWINReadVariables(uint16_t adr, uint8_t num);
void DWINWriteCurve(uint8_t mode, uint16_t num, const uint16_t *buf);
void DWINChangePage(uint16_t page);
void DWIN_Init();
void DWIN_UnInit();


#endif /* DWINDRIVER_H_ */
