// 
// 创建人： levy
// 创建时间：Jun 21, 2016
// 功能：mylib.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Jun 21, 2016                  levy          初版
// 

#include "mylib.h"
#include <mylog.h>
#include <math.h>
#include <iconv.h>
#include <time.h>
#include <sys/time.h>
//#include <linux/time.h>
#include <ctype.h>
#include <regex.h>//正则表达式
//#include <sys/timex.h>

void set_abswaittime(struct timespec*outtime, int sec, int usec)
{

	struct timeval tnow;
	gettimeofday(&tnow, NULL);
	outtime->tv_sec = tnow.tv_sec + sec;
	outtime->tv_nsec = (tnow.tv_usec + usec) * 1000;	//100ms延时
	if (outtime->tv_nsec > 1000000000)
	{
		outtime->tv_nsec %= 1000000000;
		outtime->tv_sec++;
	}
}

/*copy SRC  to DST ,SIZE is always the max size of DST, return len of SRC */
size_t my_strlcpy(char *dst, const char *src, size_t size)
{
	if (src == NULL)
		return 0;
	if (0 < size)
	{
		snprintf(dst, size, "%s", src);
		/*
		 * Platforms that lack strlcpy() also tend to have
		 * a broken snprintf implementation that doesn't
		 * guarantee nul termination.
		 *
		 * XXX: the configure script should detect and reject those.
		 */
		dst[size - 1] = '\0';
	}
	return strlen(src);
}

int my_atoi(const char *nptr)
{
	return nptr == NULL ? 0 : atoi(nptr);
}

double my_atof(const char *nptr)
{
	return (nptr == NULL) ? 0 : atof(nptr);
}

int bool_value(const char *ptr)
{
	if (ptr == NULL)
		return 0;
	if (*ptr > 1)
	{
		return atoi(ptr);
	}
	else
	{
		return *ptr;
	}

}
/*从dwin指令数据缓存区src中提取字符串到dest,最大字节数目为num
 *
 * */
int my_dwinstr(char *dest, char*src, int num)
{

	int i;
	for (i = 0; i < num; i++)
	{
		if (src[i ^ 0x01] != 0x00 && (unsigned char) (src[i ^ 0x01] & 0x80) != 0x80)
		{
			dest[i] = src[i ^ 0x01];
		}
		else
		{
			break;
		}
	}
	dest[i] = 0x00;
	return i;

}
//交换src字节顺序后存储到dest,num为dest大小
void my_x16switch(uint16_t *dest, char *src, int num)
{
	for (int i = 0; i < num; i++)
	{
		dest[i] = src[i * 2] * 0x100 + src[i * 2 + 1];
	}
}

uint8_t my_bin2bcd(uint8_t bin)
{
	return ((bin / 10) << 4) + bin % 10;
}

uint8_t my_bcd2bin(uint8_t bcd)
{
	return (bcd & 0x0f) + (bcd >> 4) * 10;
}

void my_dwinuint16cpy(void *dest, uint16_t val)
{
	char *ptr = dest;
	*ptr = (val >> 8) & 0xff;
	ptr[1] = val & 0xff;
}

int my_getdatefromstr(const char*dstr, char *obuf)
{
	regex_t reg;
	regmatch_t pm[2];
	int cflags = REG_EXTENDED, eflags = 0, z = 0;
	const char *pattern = "[0-9]{4}-[0-9]{2}-[0-9]{2}";	//"([0-9]{3}[1-9]|[0-9]{2}[1-9][0-9]{1}|[0-9]{1}[1-9][0-9]{2}|[1-9][0-9]{3})-(((0[13578]|1[02])-(0[1-9]|[12][0-9]|3[01]))|((0[469]|11)-(0[1-9]|[12][0-9]|30))|(02-(0[1-9]|[1][0-9]|2[0-8])))";
	z = regcomp(&reg, pattern, cflags);
	if (z != 0)
	{
		regfree(&reg);
		obuf[0] = 0;
		return -1;
	}
	z = regexec(&reg, dstr, 2, pm, eflags);
	if (z != 0)
	{
#if __DEBUG_MODE
		char err_buf[100];
		regerror(z, &reg, err_buf, 100);
		printf("%s\n", err_buf);

#endif
		regfree(&reg);
		obuf[0] = 0;
		return -1;
	}
	memcpy(obuf, dstr + pm[0].rm_so, (pm[0].rm_eo - pm[0].rm_so));
	obuf[pm[0].rm_eo - pm[0].rm_so + 1] = 0;
	regfree(&reg);
	return 0;
}

int my_gettimefromstr(const char*dstr, char *obuf)
{
	regex_t reg;
	regmatch_t pm[1];
	int cflags = REG_EXTENDED, eflags = 0, z = 0;
	const char *pattern = "(([0-1][0-9]|2[0-3]):[0-5][0-9])";
	z = regcomp(&reg, pattern, cflags);
	if (z != 0)
	{
		regfree(&reg);
		obuf[0] = 0;
		return -1;
	}
	z = regexec(&reg, dstr, 1, pm, eflags);
	if (z != 0)
	{

#if __DEBUG_MODE
		char err_buf[100];
		regerror(z, &reg, err_buf, 100);
		printf("%s\n", err_buf);

#endif
		regfree(&reg);
		obuf[0] = 0;
		return -1;
	}
	memcpy(obuf, dstr + pm[0].rm_so, (pm[0].rm_eo - pm[0].rm_so));
	obuf[pm[0].rm_eo - pm[0].rm_so + 1] = 0;
	regfree(&reg);
	return 0;
}

char *my_getstrfromtime_t(time_t t)
{

	static char buf[20];
	struct tm* t_now = localtime(&t);
	strftime(buf, 20, "%Y-%m-%d %H:%M:%S", t_now);
	return buf;
}

//将src标准输出到dest,最大位数max,小数位数descm,单位units,最大输出字节数num
//return 成功打印字节数，其他-1
int my_sint2str(char *dest, int src, int max, int descm, char*units, int num)
{
	if (descm > max)
		return -1;	//小数点位数大于最大位数
	int val;
	char sbuf[16];
	double f;
	int mvl = 1;
	int dvl = 1;
	for (int i = 0; i < max; i++)
	{
		mvl *= 10;
	}
	for (int i = 0; i < descm; i++)
	{
		dvl *= 10;
	}
	val = src % mvl;
	f = (double) val / dvl;
	snprintf(sbuf, 16, "%%%1d.%1df%%s", max, descm);
	return snprintf(dest, num, sbuf, f, units);
//	if(descm>0){
//		snprintf(sbuf,16,"%%1d.%%0%dd%%s",descm);
//		return snprintf(dest,num,sbuf,val/dvl,(val%dvl)<0?(val%dvl)*(-1):(val%dvl),units);
//	}else{
//		return snprintf(dest,num,"%d%s",val,units);
//	}
}

int sclope_limit(int dmin, int dmax, int smin, int smax, int value)
{
	//((double)max-(double)min)
	if (value <= smin)
	{
		return dmin;
	}
	else if (value >= smax)
	{
		return dmax;
	}
	else
	{
		double kp = ((double) dmax - (double) dmin) / ((double) smax - (double) smin);
		double vl = (double) dmin + ((double) value - (double) smin) * kp;
		return vl;
	}

}
/*
 * 清除字符串首位的 空格 \r \n字符
 * */
void clean_str(char *str)
{
	if (str == NULL || *str == 0)
		return;
	char *start = str;
	char *end = str;
	char *p = str;
	int flag = 1;
	while (*p && flag == 1)
	{
		switch (*p)
		{
			case ' ':
			case '\xff':
			case '\r':
			case '\n':
				start = ++p;
				flag = 1;
				break;
			default:
				flag = 0;
				break;
		}
	}
	if (*start == 0)
	{	//已经到字符串末尾了
		*str = 0;
		return;
	}
	end = start + strlen(start) - 1;
	flag = 1;
	p = end;
	while (flag == 1)
	{
		switch (*p)
		{
			case ' ':
			case '\xff':
			case '\r':
			case '\n':
				end = --p;
				flag = 1;
				break;
			default:
				flag = 0;
				break;
		}

	}
	end++;
	*end = 0;
	strcpy(str, start);
}
int code_convert(const char* from_c, const char* to_c, char *inbuf, int inlen, char *outbuf, int outlen)
{
	if (inlen == 0 || outlen == 0)
		return 0;
	iconv_t cd;
	size_t in_s = inlen, out_s = outlen - 1;
	char **pin = &inbuf;
	char **pout = &outbuf;
	cd = iconv_open(to_c, from_c);
	if ((uint64_t) cd == -1)
		return -1;
	size_t st = iconv(cd, pin, &in_s, pout, &out_s);
	*(*pout) = 0;
	iconv_close(cd);
	return st;
}
char *utf8Togbk_s(char * inbuf)
{
	int inlen = strlen(inbuf) + 1;
	int outlen = inlen + 1;
	char *outbuf = malloc(outlen);
	code_convert("UTF8", "GBK", inbuf, inlen, outbuf, outlen);
	memcpy(inbuf, outbuf + 2, outlen);
	free(outbuf);
	return inbuf;
}

int uft8Togbk(char *inbuf, int inlen, char*outbuf, int outlen)
{
	return code_convert("UTF8", "GBK", inbuf, inlen, outbuf, outlen);
}
int utf8Tounicode(char *inbuf, int inlen, char*outbuf, int outlen)
{
	return code_convert("utf-8", "unicode", inbuf, inlen, outbuf, outlen);
}

/*检测指定名称的进程是否存在，
 * return 0 not exist
 * 		  1 exist *
 */
int IsProcessExist(char *name)
{
	FILE* cmd;
	char buf[16];
	char cmd_buf[128];
	size_t num;
	int exist = 0;
	memset(buf, 0, sizeof(buf));
	snprintf(cmd_buf, sizeof(cmd_buf), "pidof %s", name);
	cmd = popen(cmd_buf, "r");
	memset(buf, 0, sizeof(buf));
	num = fread(buf, sizeof(char), sizeof(buf), cmd);
	if (num > 0 && strlen(buf) > 0 && atoi(buf) > 0)
	{
		exist = 1;
	}
	else
	{
		exist = 0;
	}
	pclose(cmd);
	return exist;
}
/*
 * 设置是否运行自动更新时间
 * @flag=0禁止更新
 * @flag=1允许自动更新时间
 * @return 0 sucess
 */
int SetAutoUpdateTime(int flag)
{
	FILE* cmd;
	char *cmd1 = "update-rc.d  ntp start 23 2 3 4 5 . stop 77 1 .\n";
	char *cmd2 = "update-rc.d  ntp stop 77 1 2 3 4 5 .\n";
	char *cmd3 = "update-rc.d -f ntp remove\n";
	if (flag == 0)
	{	//禁止自动更新时间
		cmd = popen("service ntp stop\n", "r");
		pclose(cmd);
		cmd = popen(cmd3, "r");
		pclose(cmd);
		cmd = popen(cmd2, "r");
		pclose(cmd);
	}
	else
	{			//允许自动更新时间
		cmd = popen(cmd3, "r");
		pclose(cmd);
		cmd = popen(cmd1, "r");
		pclose(cmd);
		cmd = popen("service ntp start\n", "r");
		pclose(cmd);
	}
	return 0;
}

int UpdateTimefromnetwork(char *server)
{
	if (server == NULL || strlen(server) == 0)
		return -1;

	char cmd_buf[128];
	snprintf(cmd_buf, sizeof(cmd_buf), "ntpdate %s", server);
	FILE* cmdf = popen(cmd_buf, "r");
	pclose(cmdf);
	return 0;
}

/*字符串拷贝函数，拷贝到字符‘c’或者结束,最大长度位n
 *
 */
char * my_strccpy(char *dest, const char *src, int c, size_t n)
{
	size_t i;

	for (i = 0; i < n && src[i] != '\0' && src[i] != c; i++)
		dest[i] = src[i];
	//for ( ; i < n; i++)
	dest[i] = '\0';

	return dest;
}
/*字符串拷贝函数，拷贝到空白符或者最大长度位n
 *
 */
char * my_strcpy_space(char *dest, const char *src, size_t n)
{
	size_t i;

	for (i = 0; i < n && src[i] != '\0' && (!isspace(src[i])); i++)
		dest[i] = src[i];
	//for ( ; i < n; i++)
	dest[i] = '\0';

	return dest;
}
/*按空白符号拆分字符串，限制最大拆分次数 n
 * @str 待拆分的字符串
 * @ptr 拆分后的字符串指针存放地址
 */
int my_strnsplit(const char * str, const char ** ptr, int n)
{
	int count;
	int notspace;
	notspace = 0;
	count = 0;
	for (size_t i = 0; str[i] != '\0' && count < n; i++)
	{
		if (!isspace(str[i]) && notspace == 0)
		{
			notspace = 1;
			ptr[count++] = &str[i];
		}
		else if (isspace(str[i]))
		{
			notspace = 0;
		}
	}
	return count;
}


