// 
// 创建人： levy
// 创建时间：Mar 1, 2017
// 功能：ts.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Mar 1, 2017                  levy          初版
// 

#include <ts.h>
#include "main.h"
#include <math.h>
static pthread_mutex_t readreg_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t readreg_cond = PTHREAD_COND_INITIALIZER;

static DwinRegDef_P D_REG_P;

//至少为buf分配48字节的大小
void  ts_printbattrytab(char * buf,Battery_cell_def*cell)
{
	cell->id=12;
	cell->volatge=381.22;
	cell->current=1.23;
	cell->temp=23.0;
	cell->res_elc=0.80;
	cell->total=102.0;
	cell->u=0.87;
	int num=snprintf(buf,48,"%3d%8.2f%7.2f%7.2f%5.0f\%%%8.2f%7.2f",
			cell->id,
			cell->volatge,
			cell->current,
			cell->temp,
			cell->res_elc*100.0,
			cell->total,
			cell->u);
	printf("all bytes is %d\n",num);
}

uint16_t ts_battery_ico_state(int res_elc)
{
	int state=0;
	if(res_elc<5)
	{
		state=0;//电量过低
	}else if(res_elc<100){
		state=(res_elc+5)/10;
	}else{
		state=11;
	}
	return state;
}

void ts_main_page_display()
{
	uint16_t buf[7];
	Battery_main_def *bms=get_battery_main_param();
	buf[0]=floor(bms->volatge*10);//电压
	buf[1]=floor(bms->current*10);//总电流
	buf[2]=floor(bms->temp*10);//温度
	buf[3]=floor(bms->res_elc*100);//剩余电量
	buf[4]=floor(bms->total);//总电量
	buf[5]=bms->relay?1:0;//充电接触器状态
	buf[6]=ts_battery_ico_state(buf[3]);//电量图标
	buf[7]=0;//充电状态指示
	buf[8]=buf[5];//充电图标状态
	if(bms->relay==0&&bms->res_elc<0.05)//电量不足
	{
		buf[7]=1;
	}else if(bms->relay&&bms->res_elc<1)//充电中
	{
		buf[7]=3;
	}else if(bms->relay&&bms->res_elc==1)//已充满
	{
		buf[7]=2;
	}
	DWINWriteVariables(0x200,9,buf);
}

void ts_display(uint16_t page)
{
	switch(page)
	{
		case 1:
			ts_main_page_display();
			break;
		default:
			break;
	}
}

int ts_readreg(DwinRegDef_P* regv, int us)
{
	struct timespec outtime;
	struct timeval tnow;
	int res = 0;
	gettimeofday(&tnow, NULL);
	outtime.tv_sec = tnow.tv_sec;
	outtime.tv_nsec = (tnow.tv_usec + us) * 1000;	//_us 延时
	if (outtime.tv_nsec >= 1000000000)
	{
		outtime.tv_nsec %= 1000000000;
		outtime.tv_sec++;
	}
	pthread_mutex_lock(&(readreg_mutex));
	DWIMReadRegs(0, 16);
	lockserial();
	res = pthread_cond_timedwait(&readreg_cond, &readreg_mutex, &outtime);
	if (res == 0)
	{
		if (regv != NULL)
			memcpy(regv, &D_REG_P, sizeof(DwinRegDef_P));
	}
	unlockserial();
	pthread_mutex_unlock(&(readreg_mutex));
	return res;
}

void ts_setregp(DWINRegDef *reg)
{
	if (reg->num == 16)
	{
		D_REG_P.Ver = reg->rbuf[0];
		D_REG_P.LED_SET = reg->rbuf[1];
		D_REG_P.BZ_TIME = reg->rbuf[2];
		D_REG_P.PCI_ID = (reg->rbuf[3] << 8) + reg->rbuf[4];
		D_REG_P.TP_FLAG = reg->rbuf[5];
		D_REG_P.TP_Status = reg->rbuf[6];
		D_REG_P.TP_Postion[0] = (reg->rbuf[7] << 8) + reg->rbuf[8];
		D_REG_P.TP_Postion[1] = (reg->rbuf[9] << 8) + reg->rbuf[0x0a];
		D_REG_P.TP_En = reg->rbuf[0x0b];
		D_REG_P.RUN_TIME = (my_bcd2bin(reg->rbuf[0x0C]) * 100 + my_bcd2bin(reg->rbuf[0x0D])) * 3600 + (my_bcd2bin(reg->rbuf[0x0E])) * 60 + my_bcd2bin(reg->rbuf[0x0F]);
		pthread_cond_signal(&readreg_cond);
	}

}

void DWINReceiveProcess(DWINVarDef *var)
{
	switch (var->saddr)
	{
		case 0x00:
			break;
		case 0x100:		//通信异常确认
			//ts_reconnect();
			break;
		default:
			break;
	}
}

void DWINRegProcess(DWINRegDef *reg)
{
	switch (reg->sreg)
	{
		case 0x00:
			ts_setregp(reg);
			break;
		case PIC_ID:
			//ts_setcurrentpage(reg);
			break;
		default:
			break;
	}
}

void *ptVarProcess(void *arp)
{
	if (pthread_detach(pthread_self()) != 0)
	{
		DEBUG_PRINTF("%s", "线程设置自身分离失败，线程是不可分离的\n");
		free(arp);
		pthread_exit("Error End Thread");
	}
	if (arp == NULL)
	{
		pthread_exit("Param is NULL");
	}
	DWINVarDef *var = (DWINVarDef *) (arp);
	DWINReceiveProcess(var);
	free(arp);
	pthread_exit("VAR");
}

void *ptRegProcess(void *arp)
{
	if (pthread_detach(pthread_self()) != 0)
	{
		DEBUG_PRINTF("%s", "线程设置自身分离失败，线程是不可分离的\n");
		free(arp);
		pthread_exit("Error End Thread");
	}
	if (arp == NULL)
	{
		pthread_exit("Param is NULL");
	}
	DWINRegDef *reg = (DWINRegDef *) (arp);
	DWINRegProcess(reg);
	free(arp);
	pthread_exit("REG");
}

void *ptTsTimeDelay(void *arg)	//定时刷新数据线程
{

	struct timeval temp;
	DEBUG_PRINTF("%s", "ptTsTimeDelay thread is created!\n");
	if (pthread_detach(pthread_self()) != 0)
	{
		DEBUG_PRINTF("%s", "线程设置自身分离失败，线程是不可分离的\n");
		pthread_exit("Error End Thread");
	}
	while (1)
	{
		temp.tv_sec = 1;
		temp.tv_usec = 0;
		select(0, NULL, NULL, NULL, &temp);
		int res = ts_readreg(NULL, 500000);	//
		if (res == ETIMEDOUT)
		{
			D_REG_P.timeoutcont++;
			if (D_REG_P.timeoutcont > 4)
			{
				ERR_LOG(NULL, "触摸屏连续超时次数过多");
			}
		}else
		{
			D_REG_P.timeoutcont=0;
		}
		ts_display(D_REG_P.PCI_ID);
	}
	pthread_exit("Error End Thread");
}
