// 
// 创建人： wnet
// 创建时间：Aug 24, 2017
// 功能：modbus_driver.c
// Copyright (c) 2016 wnet. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Aug 24, 2017                  wnet          初版
// 


#include <mylog.h>
#include <sys/time.h>
#include <pthread.h>
#include "modbus_driver.h"
#include "myserial.h"
#include <stdint.h>
#include "mylog.h"

static pthread_mutex_t md_lock_mutex = PTHREAD_MUTEX_INITIALIZER;

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

/*
 * 打开电池数据采样串口，并配置
 * */
int modbus_com_open(const char * com_name,int bandrate)
{
	int com_id;
	com_id = open(com_name, O_RDWR | O_NOCTTY); //O_NOCTTY非串口终端模式
	if (com_id > 0)
	{
		if (Set_port_attr(com_id, bandrate, 8, 'N', "1", MODBUS_VTIME, MODBUS_VMIN) == 0)
		{
			return com_id;
		}
		else
		{
			ERR_LOG("串口[%s]参数配置失败",com_name);
			close (com_id);
			return -1;
		}
	}
	else
	{
		ERR_LOG("串口[%s]打开失败\n",com_name);
		return -1;
	}

	return com_id;
}

void modbus_com_close(int com_id)
{
	close(com_id);
}




static int modbus_read(int com_id,modbus_object * obj)
{
	uint8_t rbuf[512];
	ssize_t len=0;
	fd_set rdfds;
	int max_fd=-1;
	int recv_len=MODBUS_MSG_MIN_LEN;
	int res;
	//int num = read(com_id, rbuf + remainder, 100);
	FD_ZERO(&rdfds);
	FD_SET(com_id,&rdfds);
	max_fd=com_id;
	struct timeval tv;
	bzero(rbuf,sizeof(rbuf));
	while(1)
	{
		tv.tv_sec=0;
		tv.tv_usec=100*1000;
		obj->err_state=MODBUS_OK;
		res = select(max_fd+1,&rdfds,NULL,NULL,&tv);
		if(res<0)
		{
			obj->err_state=MODBUS_DEVICE_ERR;
			res=-1;
			break;
		}else if(res ==0)
		{
			obj->err_state=MODBUS_TIME_OVER;
			res =-1;
			break;
		}else
		{
			len+= read(com_id,rbuf+len,sizeof(rbuf)-len);
			if(len>MODBUS_MSG_MIN_LEN)
			{
				if(rbuf[1]==0x03)
				{
					recv_len=rbuf[2]+5;
				}else if(rbuf[1]==0x06)
				{
					recv_len=8;
				}
				else if(rbuf[1]==0x10)
				{
					recv_len=8;
				}else if(rbuf[1]==0x00)
				{
					recv_len=6;
				}
			}
			if(len>=recv_len)
			{
				res=0;
				break;
			}
		}
	}
	if(res==0)
	{
		DEBUG_LOG("read[%d] %02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n",len,rbuf[0],rbuf[1],rbuf[2],rbuf[3],rbuf[4],rbuf[5],rbuf[6],rbuf[7],rbuf[8],rbuf[9],rbuf[10]);
		if(len<MODBUS_MSG_MIN_LEN)
		{
			obj->err_state=MODBUS_TIME_OVER;
			res=-1;
		}
		else if(rbuf[0]!=obj->addr)
		{
			res=-1;
			obj->err_state=MODBUS_ADDR_ERR;
		}
		else if(crc16_check(rbuf,len)!=0)
		{
			obj->err_state=MODBUS_CRC_ERR;
			res=-1;
		}
		else if((rbuf[1]&0x0f)!=obj->cmd)
		{
			obj->err_state=MODBUS_CMD_ERR;
			res=-1;
		}
		else if(rbuf[1]>=0x80)
		{
			obj->err_state=rbuf[2];
			res=0;
		}
		else if(rbuf[1] == 0x03)
		{
			if(obj->reg_num*2==rbuf[2])
			{
				for(int i=0;i<obj->reg_num;i++)
				{
					obj->reg_value[i]=(rbuf[i*2+3]<<8)+(rbuf[i*2+4]);
				}
				obj->err_state=MODBUS_OK;
				res=0;
			}else
			{
				obj->err_state=MODBUS_DATA_ERR;
				res=-1;
			}
		}else if(rbuf[1] == 0x6)
		{
			obj->err_state=MODBUS_OK;
			res=0;
			//write command ok!
		}else if(rbuf[1] == 0x10)
		{
			obj->err_state=MODBUS_OK;
			res=0;
		}else
		{
			obj->err_state=MODBUS_CMD_ERR;
			res=-1;
		}
	}
	return res;
}


int modebus_cmd(int com_id,modbus_object * obj)
{
	uint8_t sbuf[512];
	uint16_t crc;
	uint16_t slen;
	ssize_t res_l;
	int res;
	sbuf[0]=obj->addr;
	sbuf[1]=obj->cmd;

	switch(obj->cmd)
	{
		case 0x03:
			sbuf[2]=obj->reg_addr>>8;
			sbuf[3]=obj->reg_addr&0xff;
			sbuf[4]=0x00;
			sbuf[5]=obj->reg_num&0xff;
			crc = crc16_check(sbuf,6);
			sbuf[6]=crc&0xff;
			sbuf[7]=crc>>8;
			slen=8;
			break;
		case 0x06:
			sbuf[2]=obj->reg_addr>>8;
			sbuf[3]=obj->reg_addr&0xff;
			sbuf[4]=obj->reg_value[0]>>8;
			sbuf[5]=obj->reg_value[0]&0xff;
			crc = crc16_check(sbuf,6);
			sbuf[6]=crc&0xff;
			sbuf[7]=crc>>8;
			slen=8;
			break;
		default:
			return -1;
			break;
	}
	pthread_mutex_lock(&md_lock_mutex);

	res_l=write(com_id,sbuf,slen);
	if(res_l>0)
	{
		DEBUG_LOG("[%d]send modbus %02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n",res_l,sbuf[0],sbuf[1],sbuf[2],sbuf[3],sbuf[4],sbuf[5],sbuf[6],sbuf[7]);
		res=modbus_read(com_id,obj);
	}
	else
	{
		obj->err_state=MODBUS_DEVICE_ERR;
		res=-1;
	}
	struct timeval tv;
	tv.tv_sec=0;
	tv.tv_usec=20*1000;
	select(0,NULL,NULL,NULL,&tv);
	pthread_mutex_unlock(&md_lock_mutex);
	return res;
}





