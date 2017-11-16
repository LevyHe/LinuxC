// 
// 创建人： levy
// 创建时间：Jun 24, 2016
// 功能：MySerial.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Jun 24, 2016                  levy          初版
// 

#include "MySerial.h"

/*设置串口奇偶校验位*/
void set_parity(struct termios *opt, char parity)
{
	switch (parity)
	{
		case 'N':
		case 'n':
			opt->c_cflag &= ~PARENB;
			break;
		case 'E':
		case 'e':
			opt->c_cflag |= PARENB;
			opt->c_cflag &= ~PARODD;
			break;
		case 'O':
		case 'o':
			opt->c_cflag |= PARENB;
			opt->c_cflag |= ~PARODD;
			break;
		default:
			opt->c_cflag &= ~PARENB;
			break;
	}
}

/*设置串口数据位*/
void set_data_bit(struct termios *opt, unsigned int databit)
{
	opt->c_cflag &= ~CSIZE;
	switch (databit)
	{
		case 8:
			opt->c_cflag |= CS8;
			break;
		case 7:
			opt->c_cflag |= CS7;
			break;
		case 6:
			opt->c_cflag |= CS6;
			break;
		case 5:
			opt->c_cflag |= CS5;
			break;
		default:
			opt->c_cflag |= CS8;
			break;
	}
}

/*设置串口停止位*/
void set_stopbit(struct termios *opt, const char *stopbit)
{
	if (stopbit == NULL)
		return;
	if (0 == strcmp(stopbit, "1"))
	{
		opt->c_cflag &= ~CSTOPB;
	}
	else if (0 == strcmp(stopbit, "1.5"))
	{
		opt->c_cflag &= ~CSTOPB;
	}
	else if (0 == strcmp(stopbit, "2"))
	{
		opt->c_cflag |= CSTOPB;
	}
	else
	{
		opt->c_cflag &= ~CSTOPB;
	}
}

int set_baud_custom(int fd, int baud)
{
	int status;
	struct termios Opt;
	struct serial_struct Serial;
	tcgetattr(fd, &Opt);
	/*Get current options*/
	tcflush(fd, TCIOFLUSH);/*Flush the buffer*/
	cfsetispeed(&Opt, B38400);/*Set input speed,38400 is necessary? who can tell me why?*/
	cfsetospeed(&Opt, B38400); /*Set output speed*/
	tcflush(fd, TCIOFLUSH); /*Flush the buffer*/
	status = tcsetattr(fd, TCSANOW, &Opt); /*Set the 38400 Options*/
	if (status != 0)
	{
		perror("tcsetattr fd1");
		return COM_ATTR_ERR;
	}
	if ((ioctl(fd, TIOCGSERIAL, &Serial)) < 0)/*Get configurations vim IOCTL*/
	{
		DEBUG_PRINTF("Fail to get Serial!\n");
		return COM_CONF_ERR;
	}
	Serial.flags = ASYNC_SPD_CUST;/*We will use custom buad,May be standard,may be not */
	Serial.custom_divisor = Serial.baud_base / baud;/*In Sep4020,baud_base=sysclk/16*/
	DEBUG_PRINTF("divisor is %x\n", Serial.custom_divisor);
	if ((ioctl(fd, TIOCSSERIAL, &Serial)) < 0)/*Set it*/
	{
		DEBUG_PRINTF("Fail to set Serial\n");
		return COM_CONF_ERR;
	}
	return COM_OK;
}
int set_baudrate(struct termios *opt, unsigned int baudrate)
{
	int baud = 0;
	switch (baudrate)
	{
		case 50:
			baud = B50;
			break;
		case 75:
			baud = B75;
			break;
		case 110:
			baud = B110;
			break;
		case 134:
			baud = B134;
			break;
		case 150:
			baud = B150;
			break;
		case 200:
			baud = B200;
			break;
		case 300:
			baud = B300;
			break;
		case 600:
			baud = B600;
			break;
		case 1200:
			baud = B1200;
			break;
		case 1800:
			baud = B1800;
			break;
		case 2400:
			baud = B2400;
			break;
		case 4800:
			baud = B4800;
			break;
		case 9600:
			baud = B9600;
			break;
		case 19200:
			baud = B19200;
			break;
		case 38400:
			baud = B38400;
			break;
		case 57600:
			baud = B57600;
			break;
		case 115200:
			baud = B115200;
			break;
		case 230400:
			baud = B230400;
			break;
		case 460800:
			baud = B460800;
			break;
		case 500000:
			baud = B500000;
			break;
		case 576000:
			baud = B576000;
			break;
		case 921600:
			baud = B921600;
			break;
		case 1000000:
			baud = B1000000;
			break;
		case 1152000:
			baud = B1152000;
			break;
		case 1500000:
			baud = B1500000;
			break;
		case 2000000:
			baud = B2000000;
			break;
		case 2500000:
			baud = B2500000;
			break;
		case 3000000:
			baud = B3000000;
			break;
		case 3500000:
			baud = B3500000;
			break;
		case 4000000:
			baud = B4000000;
			break;
		case 0:
			baud = 0;
			break;
		default:
			break;
	}
	if (baud >= 0)
	{
		cfsetispeed(opt, baud);
		cfsetospeed(opt, baud);
	}
	else if (baudrate > 0)
	{
		baud = baudrate;
	}
	return baud;
}
struct serial_struct Serial;
struct termios opt;
/*配置串口属性，fd串口描述符，baudrate,数据位，奇偶校验，停止位，超时时间，最小接收字节数*/
int Set_port_attr(int fd, int baudrate, int databit, char parity, const char *stopbit, int vtime, int vmin)
{

	int baud;

	if (tcgetattr(fd, &opt) != 0)
		return COM_CONF_ERR;
	baud = set_baudrate(&opt, baudrate);
	opt.c_cflag |= (CLOCAL | CREAD); //使能接收与本地模式
	set_data_bit(&opt, databit);
	set_parity(&opt, parity);
	set_stopbit(&opt, stopbit);
	opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //设置原始输入模式
	opt.c_oflag &= ~OPOST; //设置原始输出模式
	opt.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); //取消屏蔽特殊符号
	opt.c_cc[VTIME] = vtime;
	opt.c_cc[VMIN] = vmin;
	tcflush(fd, TCIOFLUSH); // 清除所有正在发生的I/O数据。
	if (tcsetattr(fd, TCSANOW, &opt) != 0)
		return COM_CONF_ERR; //使设置生效
	if (baud == baudrate && baudrate != 0)
	{
		if (set_baud_custom(fd, baudrate) != 0)
			return COM_CONF_ERR;
	}
//	if ((ioctl(fd, TIOCGSERIAL, &Serial)) < 0)/*Get configurations vim IOCTL*/{
//			DEBUG_PRINTF("Fail to get Serial!\n");
//			return COM_CONF_ERR;
//		}
	return COM_OK;
}

/*
 void set_baud(int fd, int baud) {
 int status;
 struct termios Opt;
 struct serial_struct Serial;
 tcgetattr(fd, &Opt);
 Get current options
 tcflush(fd, TCIOFLUSH);Flush the buffer
 cfsetispeed(&Opt, B38400);Set input speed,38400 is necessary? who can tell me why?
 cfsetospeed(&Opt, 38400); Set output speed
 tcflush(fd, TCIOFLUSH); Flush the buffer
 status = tcsetattr(fd, TCSANOW, &Opt); Set the 38400 Options
 if (status != 0) {
 perror("tcsetattr fd1");
 return;
 }
 if ((ioctl(fd, TIOCGSERIAL, &Serial)) < 0)Get configurations vim IOCTL{
 printf("Fail to get Serial!\n");
 return;
 }
 Serial.flags = ASYNC_SPD_CUST;We will use custom buad,May be standard,may be not
 Serial.custom_divisor = Serial.baud_base / baud;In Sep4020,baud_base=sysclk/16
 printf("divisor is %x\n", Serial.custom_divisor);
 if ((ioctl(fd, TIOCSSERIAL, &Serial)) < 0)Set it{
 printf("Fail to set Serial\n");
 return;
 }
 ioctl(fd, TIOCGSERIAL, &Serial);Get it again,not necessary.
 printf("\nBAUD: success set baud to %d,custom_divisor=%d,baud_base=%d\n",
 baud, Serial.custom_divisor, Serial.baud_base);
 }*/

