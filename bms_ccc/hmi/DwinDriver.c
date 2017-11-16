
// 
// 创建人： levy
// 创建时间：Jul 21, 2016
// 功能：DwinDriver.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Jul 21, 2016                  levy          初版
// 

#include <MySerial.h>
//#include "touchscreen.h"
#include <mylog.h>
//#include <mysqlassist.h>
#include <sys/time.h>
#include <pthread.h>
#include "DwinDriver.h"
#include <app/bms_def.h>

#define MAXSENDBUFSIZE 512

const char WriteRegCmd = 0x80;
const char ReadRegCmd = 0x81;

const char WriteVariableCmd = 0x82;
const char ReadVariableCmd = 0x83;

const char WriteCurveCmd = 0x84;

const uint8_t Header1 = 0x5A;
const uint8_t Header2 = 0xA5;
const char *headerstr = "\x5A\xA5";

static char Comname[32];
static char Comband[16];
static int BandRate;
static int Com_ID;
static int IsComOK = 0;
static uint8_t rbuf[512];
static int remainder = 0; //缓存区中剩余字节数

static pthread_t thTsProcess;
static pthread_mutex_t serialsendmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t dwincmdmutex = PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t dwinlockserial=PTHREAD_MUTEX_INITIALIZER;
//static int isseriallock=0;
int read_com_conf()
{
	char * confilename = getenv("CONFIG_FILE");
	if (confilename == NULL)
	{
		ERR_THREW("CONFIG_FILE环境变量不存在");
		return -1;
	}
	if (access(confilename, F_OK) == -1)
	{
		ERR_THREW("%s文件不存在", confilename);
		return -1;
	}
	char fname[16];
	if (GetProfileString(confilename, "touchscreen", "com", fname) != 0) //用户名读取失败
	{
		return USR_CONF_FAIL;
	}
	else
	{
		snprintf(Comname, 32, "/dev/%s", fname);
	}
	if (GetProfileString(confilename, "touchscreen", "baudrate", Comband) != 0) //端口读取失败
	{
		BandRate = 115200;
	}
	else
	{
		BandRate = my_atoi(Comband);
	}
	return USR_SUCESS;
}
//锁定串口禁止其它线程继续往串口写数据
inline void lockserial()
{
	pthread_mutex_lock(&serialsendmutex);
}
//释放串口，允许继续写数据
inline void unlockserial()
{
	pthread_mutex_unlock(&serialsendmutex);
}
////检查串口是否被锁定，并等待
//void waitseriallock(){
//	pthread_mutex_lock(&serialsendmutex);
//}

int com_start()
{

	if (read_com_conf() != 0)
	{ //服务器配置文件读取失败
		ERR_THREW("串口配置文件读取失败\n");
		return USR_CONF_FAIL;
	}
	Com_ID = open(Comname, O_RDWR | O_NOCTTY); //O_NOCTTY非串口终端模式
	if (Com_ID > 0)
	{
		DEBUG_PRINTF("串口打开成功\n");
		if (Set_port_attr(Com_ID, BandRate, 8, 'N', "1", DWIN_VTIME, DWIN_VMIN) == 0)
		{
			DEBUG_PRINTF("串口参数配置成功\n");
			IsComOK = TRUE;
		}
		else
		{
			ERR_THREW("串口参数配置失败\n");
			com_close();
			return USR_COM_ERR;
		}
	}
	else
	{
		DEBUG_PRINTF("串口打开失败\n");
		return USR_COM_ERR;
	}

	return USR_SUCESS;
}

void com_close(void)
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
/*从缓存区中读取dwinz指令
 * cmd 读取到的指令
 * buf 缓存区
 * num 缓存区中可用字节数
 **/
int GetDWINCmd(DWINCmdDef*cmd, uint8_t *buf, int num)
{

	for (int i = 0; i < num - 1; i++)
	{
		if (buf[i] == Header1 && buf[i + 1] == Header2)
		{ //找到祯头
			int len = buf[i + 2];
			if ((num - len - i - 3) < 0) //缓存区中字节数不足
				break;
			cmd->header1 = buf[i];
			cmd->header2 = buf[i + 1];
			cmd->len = len;
			cmd->cmd = buf[i + 3];
			memcpy(cmd->buf, buf + 4, len);
			if ((num - len - i - 3) > 0) //缓存区中仍有可用字节
			{
				remainder = num - len - i - 3;
				memcpy(rbuf, buf + len + i + 3, remainder); //将剩余字节移到缓存去头

			}
			else
			{
				remainder = 0;
			}
			return 0;	//获得完整指令
		}
	}
	if (remainder > 256)
	{	//缓存区中无效字节太多，清空缓存区
		remainder = 0;
	}
	return -1;
}

int GetDWINVar(DWINVarDef*var, uint8_t * buf, int num)
{
	var->saddr = (buf[0] << 8) + buf[1];
	int len = buf[2];
	var->vnum = len;
	if ((num - 3 - len * 2) < 0)
	{
		return -1;
	}
	for (int i = 0; i < len; i++)
	{
		var->vbuf[i] = (buf[i * 2 + 3] << 8) + buf[i * 2 + 4];
	}
	return USR_SUCESS;
}

int GetDWINReg(DWINRegDef *reg, uint8_t *buf, int num)
{
	reg->sreg = buf[0];
	int len = buf[1];
	reg->num = len;
	if ((num - 2 - len) < 0)
	{
		return -1;
	}
	memcpy(reg->rbuf, buf + 2, len);
	return USR_SUCESS;
}

void DWINWriteReg(uint8_t reg, uint8_t value)
{

	pthread_mutex_lock(&serialsendmutex);
	char *sbuf = malloc(MAXSENDBUFSIZE);
	int size;
	sbuf[0] = Header1;
	sbuf[1] = Header2;
	sbuf[2] = 3;
	sbuf[3] = WriteRegCmd;
	sbuf[4] = reg;
	sbuf[5] = value;
	size = 6;
	write(Com_ID, sbuf, size);
	free(sbuf);
	pthread_mutex_unlock(&serialsendmutex);

}

void DWINWriteRegs(uint8_t reg, uint8_t num, const uint8_t *buf)
{

	pthread_mutex_lock(&serialsendmutex);
	char *sbuf = malloc(MAXSENDBUFSIZE);
	int size;
	sbuf[0] = Header1;
	sbuf[1] = Header2;
	sbuf[2] = 2 + num;
	sbuf[3] = WriteRegCmd;
	sbuf[4] = reg;
	memcpy(sbuf + 5, buf, num);
	size = 5 + num;
	write(Com_ID, sbuf, size);
	free(sbuf);
	pthread_mutex_unlock(&serialsendmutex);

}

void DWIMReadRegs(uint8_t addr, uint8_t num)
{

	pthread_mutex_lock(&serialsendmutex);
	char *sbuf = malloc(MAXSENDBUFSIZE);
	int size;
	sbuf[0] = Header1;
	sbuf[1] = Header2;
	sbuf[2] = 3;
	sbuf[3] = ReadRegCmd;
	sbuf[4] = addr;
	sbuf[5] = num;
	size = 6;
	write(Com_ID, sbuf, size);
	free(sbuf);
	pthread_mutex_unlock(&serialsendmutex);

}

void DWINWriteVariable(uint16_t adr, const uint16_t value)
{

	pthread_mutex_lock(&serialsendmutex);
	char *sbuf = malloc(MAXSENDBUFSIZE);
	int size;
	sbuf[0] = Header1;
	sbuf[1] = Header2;
	sbuf[2] = 5;
	sbuf[3] = WriteVariableCmd;
	sbuf[4] = (adr >> 8) & 0xff;
	sbuf[5] = adr & 0xff;
	sbuf[6] = (value >> 8) & 0xff;
	sbuf[7] = value & 0xff;
	size = 8;
	write(Com_ID, sbuf, size);
	free(sbuf);
	pthread_mutex_unlock(&serialsendmutex);

}

void DWINWriteVariables(uint16_t adr, uint8_t num, uint16_t *buf)
{

	pthread_mutex_lock(&serialsendmutex);
	char *sbuf = malloc(MAXSENDBUFSIZE);
	int size;
	num = num & 0x7F;
	sbuf[0] = Header1;
	sbuf[1] = Header2;
	sbuf[2] = 3 + num * 2;
	sbuf[3] = WriteVariableCmd;
	sbuf[4] = (adr >> 8) & 0xff;
	sbuf[5] = adr & 0xff;
	for (int i = 0; i < num; i++)
	{
		sbuf[i * 2 + 6] = (buf[i] >> 8) & 0xff;
		sbuf[i * 2 + 7] = buf[i] & 0xff;
	}
	size = 6 + num * 2;
	write(Com_ID, sbuf, size);
	free(sbuf);
	pthread_mutex_unlock(&serialsendmutex);

}

//向DGUS发送buf中内容，大小为num字节,num应为偶数
void DWINWriteVararry(uint16_t adr, uint8_t num, uint8_t*buf)
{

	pthread_mutex_lock(&serialsendmutex);
	char *sbuf = malloc(MAXSENDBUFSIZE);
	int size;

	sbuf[0] = Header1;
	sbuf[1] = Header2;
	sbuf[2] = 3 + num;
	sbuf[3] = WriteVariableCmd;
	sbuf[4] = (adr >> 8) & 0xff;
	sbuf[5] = adr & 0xff;
	for (int i = 0; i < num; i++)
	{
		sbuf[i + 6] = buf[i];
	}
	size = 6 + num;
	write(Com_ID, sbuf, size);
	free(sbuf);
	pthread_mutex_unlock(&serialsendmutex);

}

void DWINReadVariables(uint16_t adr, uint8_t num)
{

	pthread_mutex_lock(&serialsendmutex);
	char *sbuf = malloc(MAXSENDBUFSIZE);
	int size;
	sbuf[0] = Header1;
	sbuf[1] = Header2;
	sbuf[2] = 4;
	sbuf[3] = ReadVariableCmd;
	sbuf[4] = (adr >> 8) & 0xff;
	sbuf[5] = adr & 0xff;
	sbuf[6] = num & 0x7f;
	size = 7;
	write(Com_ID, sbuf, size);
	free(sbuf);
	pthread_mutex_unlock(&serialsendmutex);

}

void DWINWriteCurve(uint8_t mode, uint16_t num, const uint16_t *buf)
{

	pthread_mutex_lock(&serialsendmutex);
	char *sbuf = malloc(MAXSENDBUFSIZE);
	int size;
	sbuf[0] = Header1;
	sbuf[1] = Header2;
	sbuf[2] = mode;
	for (int i = 0; i < num; i++)
	{
		sbuf[i * 2 + 3] = (buf[i] >> 8) & 0xff;
		sbuf[i * 2 + 4] = buf[i] & 0xff;
	}
	size = 3 + num * 2;
	write(Com_ID, sbuf, size);
	free(sbuf);
	pthread_mutex_unlock(&serialsendmutex);

}

void DWINChangePage(uint16_t page)
{
	uint8_t xbuf[2];
	xbuf[0] = page >> 8;
	xbuf[1] = page & 0xff;
	DWINWriteRegs(PIC_ID, 2, xbuf);
}

void DWIN_Init()
{
	int res;
	res = com_start();
	if (res != USR_SUCESS)
	{
		ERR_LOG(NULL, "串口打开失败，请检查串口是否可用");
		exit(1);
	}
	res = pthread_create(&thTsProcess, NULL, ptTsProcess, NULL);	//创建串口触摸屏程序接收线程
	if (res != 0)
	{
		ERR_LOG(NULL, "ptTsProcess creat faild");
		exit(1);
	}
}

void DWIN_UnInit()
{
	com_close();
}

void *ptTsProcess(void *arp)
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

			ERR_MSG("com is not open");
			ERR_LOG(NULL, "串口没有打开进程无法创建");
			exit(1);
		}
		int num = read(Com_ID, rbuf + remainder, DWIN_VMIN);

		remainder += num;
		if (remainder < 0)
		{
			ERR_LOG(NULL, "串口接收数据异常");
			exit(1);
		}
		if (remainder > DWIN_MinMsgLen)
		{
			flag = 1;
			DWINCmdDef *cmd = malloc(sizeof(DWINCmdDef));
			if (cmd == NULL)
			{
				ERR_MSG("内存分配失败");
			}
			do
			{
				pthread_mutex_lock(&dwincmdmutex);
				if (GetDWINCmd(cmd, rbuf, remainder) == 0)//获得完整指令
				{
					switch (cmd->cmd)
					{
						case 0x80:			//写指令
							break;
						case 0x81:			//读寄存器应答指令
						{
							DWINRegDef *reg1 = malloc(sizeof(DWINRegDef));
							if (GetDWINReg(reg1, (uint8_t*) cmd->buf, cmd->len - 1) == 0)
							{
								//DWINRegProcess(reg1);
								pthread_t thregprocess;
								int res = pthread_create(&thregprocess, NULL, ptRegProcess, reg1);
								if (res != 0)
								{
									DEBUG_PRINTF("ptRegProcess creat faild\n");
								}
							}
							//free(reg1);
						}
							break;
						case 0x82:			//写指令
							break;
						case 0x83:			//读变量应答指令或触摸按键下发指令
						{
							DWINVarDef *var1 = malloc(sizeof(DWINVarDef));
							if (GetDWINVar(var1, cmd->buf, cmd->len - 1) == 0)
							{
								//DWINReceiveProcess(var1);
								pthread_t thvarprocess;
								int res = pthread_create(&thvarprocess, NULL, ptVarProcess, var1);
								if (res != 0)
								{
									DEBUG_PRINTF("ptVarProcess creat faild\n");
								}
							}

							//free(var1);
						}
							break;
						default:
							break;
					}

				}
				else
				{
					flag = 0;			//跳出循环
				}
				pthread_mutex_unlock(&dwincmdmutex);
			} while (flag);
			free(cmd);
		}

	}
	pthread_exit("Error End Thread");
}

//这是一个弱定义函数，请用同名函数替代
__attribute__((weak)) void *ptVarProcess(void *arp)
{
	if (pthread_detach(pthread_self()) != 0)
	{
		DEBUG_PRINTF("%s", "线程设置自身分离失败，线程是不可分离的\n");
		pthread_exit("Error End Thread");
	}

	//  if you redefine the ptRegProcess function must free the arp
	free(arp);
	pthread_exit("This is the weak function");
}
//这是一个弱定义函数，请用同名函数替代
__attribute__((weak)) void *ptRegProcess(void *arp)
{
	if (pthread_detach(pthread_self()) != 0)
	{
		DEBUG_PRINTF("%s", "线程设置自身分离失败，线程是不可分离的\n");
		pthread_exit("Error End Thread");
	}

//  if you redefine the ptRegProcess function must free the arp
	free(arp);
	pthread_exit("This is the weak function");
}


