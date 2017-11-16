// 
// 创建人： levy
// 创建时间：Jun 21, 2016
// 功能：mylib.h
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Jun 21, 2016                  levy          初版
// 

#ifndef MYLIB_H_
#define MYLIB_H_

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>

//定义读取配置文件时一行的最大长度
#define LINE_LEN 256

extern void set_abswaittime(struct timespec*outtime, int sec, int usec);
extern size_t my_strlcpy(char *dst, const char *src, size_t size);
extern int my_atoi(const char *nptr);
extern double my_atof(const char *nptr);
extern int bool_value(const char *ptr);
extern int my_dwinstr(char *dest, char*src, int num);
extern void my_x16switch(uint16_t *dest, char *src, int num);
extern uint8_t my_bin2bcd(uint8_t bin);
extern uint8_t my_bcd2bin(uint8_t bcd);
extern char * my_strccpy(char *dest, const char *src, int c, size_t n);
extern char * my_strcpy_space(char *dest, const char *src, size_t n);
extern int my_strnsplit(const char * str, const char ** ptr, int n);

extern int my_sint2str(char *dest, int src, int max, int descm, char*units, int num);
extern int sclope_limit(int dmin, int dmax, int smin, int smax, int value);
extern void clean_str(char *str);
extern int isipv4addr(const char *str);
extern int SetIfAddr(const char *ifname, const char *Ipaddr, const char *mask);
extern int GetIfgetway_PP(const char *ifname, char *gateway);
extern int GetHwaddr_PP(const char * ifname, char *hwaddr);
extern int GetIpnetmask_PP(const char * ifname, char *netmask);
extern int GetIpaddress_PP(const char * ifname, char *ipaddr);
extern int GetIfdefaultdns_PP(char *dns);

extern int SetIpConfigFlie_DHCP(const char *ifname, int dhcp);
extern int SetIpConfigFlie_Signle(const char *ifname, const char *IpKey, const char *Ipvalue);
extern int SetIpConfigFlie(const char *ifname, const char *Ipaddr, const char *mask, const char *gateway);
extern int GetIpConfigfileValue(const char *ifname, const char *IpKey, char *Ipvalue);
extern int NetworkReboot(const char *ifname);

extern int uft8Togbk(char *inbuf, int inlen, char*outbuf, int outlen);
extern int utf8Tounicode(char *inbuf, int inlen, char*outbuf, int outlen);
extern char *utf8Togbk_s(char * inbuf);
extern void my_dwinuint16cpy(void *dest, uint16_t val);
extern int my_getdatefromstr(const char*dstr, char *obuf);
extern int my_gettimefromstr(const char*dstr, char *obuf);
extern char *my_getstrfromtime_t(time_t t);
extern int IsProcessExist(char *name);
extern int system_reboot();
extern int system_poweroff();
extern int SetAutoUpdateTime(int flag);
extern int UpdateTimefromnetwork(char *server);

extern uint16_t Crc16_check(uint8_t *pDataIn, uint16_t iLenIn);


#define GET_ARRAY_LEN(type) (sizeof(type)/sizeof(type[0]))
#define UTF8STR_TO_GBK(instr,outbuf) uft8Togbk(instr,strlen(instr),outbuf,sizeof(outbuf))

#endif /* MYLIB_H_ */
