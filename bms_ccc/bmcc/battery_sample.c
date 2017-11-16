// 
// 创建人： levy
// 创建时间：Mar 23, 2017
// 功能：battery_sample.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Mar 23, 2017                  levy          初版
// 

#include <MySerial.h>
#include <mylog.h>
#include <sys/time.h>
#include <pthread.h>
#include "battery_sample.h"
#include <stdint.h>

static char Comname[32];
static char Comband[16];
static int BandRate;
static int Com_ID;
static int IsComOK = 0;
static uint8_t rbuf[512];
static int remainder = 0; //缓存区中剩余字节数
static pthread_mutex_t battery_sample_mutex = PTHREAD_MUTEX_INITIALIZER;

const uint8_t s_header1 = 0x5A;
const uint8_t s_header2 = 0xA5;

/*
 * 打开电池数据采样串口，并配置
 * */
int sample_com_init()
{
	char * confilename = getenv("CONFIG_FILE");
	if (confilename == NULL)
	{
		ERR_LOG("BMS","CONFIG_FILE环境变量不存在");
		return -1;
	}
	if (access(confilename, F_OK) == -1)
	{
		ERR_LOG("BMS","%s文件不存在", confilename);
		return -1;
	}
	char fname[16];
	if (GetProfileString(confilename, "battrry_sample", "com", fname) != 0) //用户名读取失败
	{
		ERR_LOG("BMS","battrry_sample.com 配置无效");
		return -1;
	}
	else
	{
		snprintf(Comname, 32, "/dev/%s", fname);
	}
	if (GetProfileString(confilename, "battrry_sample", "baudrate", Comband) != 0) //端口读取失败
	{
		BandRate = 115200;
	}
	else
	{
		BandRate = my_atoi(Comband);
	}
	Com_ID = open(Comname, O_RDWR | O_NOCTTY); //O_NOCTTY非串口终端模式
	if (Com_ID > 0)
	{
		DEBUG_PRINTF("串口打开成功\n");
		if (Set_port_attr(Com_ID, BandRate, 8, 'N', "1", BATTERY_SAMPLE_VTIME, BATTERY_SAMPLE_VMIN) == 0)
		{
			DEBUG_PRINTF("串口参数配置成功\n");
			IsComOK = 1;
			return 0;
		}
		else
		{
			ERR_LOG("BMS","串口%s参数配置失败\n",Comname);
			com_close();
			return -1;
		}
	}
	else
	{
		ERR_LOG("BMS","串口%s打开失败\n",Comname);
		return -1;
	}

	return 0;
}

void sample_com_close(void)
{
	if (Com_ID <= 0)
	{
		IsComOK = 0;
		Com_ID = 0;
	}
	else
	{
		close(Com_ID);
		IsComOK = 0;
		Com_ID = 0;
	}
}

int sample_data_unpackage(sample_package_def *cmd,uint8_t*buf ,int num)
{
	for(int i=0;i<num;i++)
	{
		if (buf[i] == s_header1 && buf[i + 1] == s_header2)
		{
			uint16_t len = (buf[i+3]<<8)+buf[i+4];
			if ((num - len - i - 7) < 0) //缓存区中字节数不足
				break;
			if(Crc16_check(buf+i,len+7)!=0)//crc校验
			{
				remainder = num - len - i - 7;
				break;
			}
			cmd->header1=buf[i];
			cmd->header2=buf[i+1];
			cmd->cmd_code=buf[i+2];
			cmd->msg_len=len;
			cmd->uint_num=buf[i+5];
			cmd->uint_array[0]=&buf[i+6];
			int j;
			for(j=1;j<cmd->uint_num;j++)
			{
				cmd->uint_array[j]=cmd->uint_array[j-1]+cmd->uint_array[j-1][1]+2;
				if(cmd->uint_array[j]>=buf+len+5+i)//当前数据包地址已经超出缓存区
				{
					break;
				}
			}
			if(j!=j<cmd->uint_num) //参数组数据存在异常
			{
				remainder = num - len - i - 7;
				break;
			}
			if ((num - len - i - 3) > 0) //缓存区中仍有可用字节
			{
				remainder = num - len - i - 7;
				memcpy(rbuf, buf + len + i + 7, remainder); //将剩余字节移到缓存区头

			}
			else
			{
				remainder = 0;
			}
			return 0;
		}
	}
	if (remainder >= sizeof(rbuf))
	{	//缓存区中无效字节太多，清空缓存区
		remainder = 0;
	}
	return -1;
}

void *ptsample_com_process(void *arp)
{

	if (pthread_detach(pthread_self()) != 0)
	{
		DEBUG_PRINTF("%s", "线程设置自身分离失败，线程是不可分离的\n");
	}
	int flag = 0;
	while (1)
	{
		if (Com_ID <= 0)
		{
			ERR_LOG("BMS", "串口没有打开进程无法创建");
			exit(1);
		}
		int num = read(Com_ID, rbuf + remainder, BATTERY_SAMPLE_VMIN);

		remainder += num;
		if (remainder < 0)
		{
			ERR_LOG(NULL, "串口接收数据异常");
			exit(1);
		}
		if (remainder > BATTERY_SAMPLE_MSG_MIN_LEN)
		{
			flag = 1;

			do
			{
				pthread_mutex_lock(&battery_sample_mutex);
				sample_package_def *cmd = malloc(sizeof(sample_package_def));
				if(sample_data_unpackage(cmd,rbuf,remainder)==0)
				{
					if(cmd->cmd_code==0x0A)//有效的数据包
					{

					}
				}else
				{
					flag=0;
				}
				free(cmd);
				pthread_mutex_unlock(&battery_sample_mutex);
			} while (flag);

		}

	}
	pthread_exit("Error End Thread");
}

