// 
// 创建人： levy
// 创建时间：Jun 24, 2016
// 功能：MySerial.h
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Jun 24, 2016                  levy          初版
// 

#ifndef MYSERIAL_H_
#define MYSERIAL_H_
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <conf.h>
#include <pthread.h>
#include <sys/time.h>
//#include <linux/termios.h>
#include <asm-generic/ioctls.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/serial.h>
#include <mylog.h>
//#include <vdgs_def.h>
#include <stdint.h>
#include <termios.h>

typedef enum
{
	COM_OK = 0, COM_CONF_ERR, COM_BAUD_ERR, COM_ATTR_ERR
} COM_ERR_TYPE;


/*配置串口属性，fd串口描述符，baudrate,奇偶校验，停止位，数据位，超时时间，最小接收字节数*/
extern int Set_port_attr(int fd, int baudrate, int databit, char parity, const char *stopbit, int vtime, int vmin);
void com_close(void);
void unlockserial();
void lockserial();
#endif /* MYSERIAL_H_ */
