// 
// 创建人： levy
// 创建时间：2016-6-5
// 功能：conf.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期                                   负责人                           变更内容
// ───────────────────────────────────────────
// V0.01         2016-6-5                  levy          初版
// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <conf.h>
#include <ctype.h>

/*   删除左边的空格   */
char * l_trim(char * szOutput, const char *szInput)
{
	assert(szInput != NULL);
	assert(szOutput != NULL);
	assert(szOutput != szInput);
	for (; *szInput != '\0' && isspace(*szInput); ++szInput)
	{

	}
	return strcpy(szOutput, szInput);
}

/*   删除右边的空格   */
char *r_trim(char *szOutput, const char *szInput)
{
	char *p = NULL;
	assert(szInput != NULL);
	assert(szOutput != NULL);
	assert(szOutput != szInput);
	strcpy(szOutput, szInput);
	for (p = szOutput + strlen(szOutput) - 1; p >= szOutput && isspace(*p); --p)
	{
		;
	}
	*(++p) = '\0';
	return szOutput;
}

/*   删除两边的空格   */
char * a_trim(char * szOutput, const char * szInput)
{
	char *p = NULL;
	assert(szInput != NULL);
	assert(szOutput != NULL);
	l_trim(szOutput, szInput);
	for (p = szOutput + strlen(szOutput) - 1; p >= szOutput && isspace(*p); --p)
	{
		;
	}
	*(++p) = '\0';
	return szOutput;
}
/**读取配置文件参数
 *@profile：文件完整路径名称
 *@AppName：配置分组名称即[]内的关键字名称
 *@KeyName：关键字名称
 *@KeyVal：读取的建值返回值
 *@return：结果返回0=OK,-1=ERROR
 */
int GetProfileString(char *profile, char *AppName, char *KeyName, char *KeyVal)
{
	char app_buf[32], key_buf[32];
	char *buf_p, *c;
	char buf_i[KEYVALLEN], buf_o[KEYVALLEN];
	char KeyVal_o[KEYVALLEN], KeyVal_r[KEYVALLEN];
	FILE *fp;
	int found = 0; /* 1 AppName 2 KeyName */
	if ((fp = fopen(profile, "r")) == NULL)
	{
		//文件打开失败
		printf("openfile [%s] error [%s]\n", profile, strerror(errno));
		return (-1);
	}
	fseek(fp, 0, SEEK_SET);
	memset(app_buf, 0, sizeof(app_buf));
	sprintf(app_buf, "[%s]", AppName);

	while (!feof(fp) && fgets(buf_i, KEYVALLEN, fp) != NULL)
	{
		if (strlen(buf_o) >= (KEYVALLEN - 1)) //字符串长度过长,可能不包含‘\n’
			continue;
		l_trim(buf_o, buf_i);
		if (strlen(buf_o) <= 0)
			continue;
		buf_p = buf_o;

		if (found == 0)
		{
			if (buf_p[0] != '[')
			{
				continue;
			}
			else if (strncmp(buf_p, app_buf, strlen(app_buf)) == 0)
			{
				found = 1;
				continue;
			}

		}
		else if (found == 1)
		{
			if (buf_p[0] == '#')
			{
				continue;
			}
			else if (buf_p[0] == '[')
			{
				break;
			}
			else
			{
				if ((c = (char*) strchr(buf_p, '=')) == NULL)
					continue;
				memset(key_buf, 0, sizeof(key_buf));
				sscanf(buf_p, "%[^=|^ |^\t]", key_buf); //获取'='左边的keyname
				if (strcmp(key_buf, KeyName) == 0)
				{
					memset(KeyVal_r, 0, sizeof(KeyVal_r));
					sscanf(++c, "%[^\n]", KeyVal_r); //获取'='右边的keyvalue
					a_trim(KeyVal_o, KeyVal_r);
					strcpy(KeyVal, KeyVal_o);
					found = 2;
					break;
				}
				else
				{
					continue;
				}
			}
		}
	}
	fclose(fp);
	if (found == 2)
		return (0);
	else
		return (-1);
}
/**更改配置文件配置参数,只修改已有的键值
 *@profile：文件完整路径名称
 *@AppName：配置分组名称即[]内的关键字名称
 *@KeyName：关键字名称
 *@KeyVal：要写入的建值
 *@return：结果返回0=OK,-1=ERROR
 */
int SetConfigFile(char *profile, char *AppName, char *KeyName, char *KeyVal)
{
	char app_buf[32], key_buf[32];
	char *buf_p, *c;
	char buf_i[KEYVALLEN], buf_o[KEYVALLEN];
	char KeyVal_o[KEYVALLEN], KeyVal_r[KEYVALLEN];
	char Keyline[KEYVALLEN];
	FILE *fp;
	int found = 0; /* 1 AppName 2 KeyName */
	if ((fp = fopen(profile, "r")) == NULL)
	{
		//文件打开失败
		perror("config file open error:");
		return (-1);
	}
	fseek(fp, 0, SEEK_END);
	int fsize = ftell(fp); //读取文件长度
	fseek(fp, 0, SEEK_SET);
	int wcount = 0, wlen = fsize + KEYVALLEN;
	char * wbuf = malloc(wlen);
	memset(wbuf, 0, wlen);
	sprintf(app_buf, "[%s]", AppName);
	while (!feof(fp) && fgets(buf_i, KEYVALLEN, fp) != NULL)
	{
		if (strlen(buf_i) >= (KEYVALLEN - 1)) //字符串长度过长,可能不包含‘\n’
		{
			wcount += snprintf(wbuf + wcount, wlen - wcount, "%s", buf_i);
			continue;
		}
		l_trim(buf_o, buf_i);
		if (strlen(buf_o) <= 0)
		{
			wcount += snprintf(wbuf + wcount, wlen - wcount, "%s", buf_i);
			continue;
		}
		buf_p = buf_o;

		if (found == 0)
		{
			wcount += snprintf(wbuf + wcount, wlen - wcount, "%s", buf_i);
			if (buf_p[0] != '[')
			{
				continue;
			}
			else if (strncmp(buf_p, app_buf, strlen(app_buf)) == 0)
			{
				found = 1;
				continue;
			}
		}
		else if (found == 1)
		{
			if (buf_p[0] == '#')
			{
				wcount += snprintf(wbuf + wcount, wlen - wcount, "%s", buf_i);
				continue;
			}
			else if (buf_p[0] == '[')
			{ //配置文件格式存在错误
				break;
			}
			else
			{
				if ((c = (char*) strchr(buf_p, '=')) == NULL) //不包含‘=’无效的行
				{
					wcount += snprintf(wbuf + wcount, wlen - wcount, "%s", buf_i);
					continue;
				}
				memset(key_buf, 0, sizeof(key_buf));

				sscanf(buf_p, "%[^=|^ |^\t]", key_buf); //获取'='左边的keyname
				if (strcmp(key_buf, KeyName) == 0)
				{
					memset(KeyVal_r, 0, sizeof(KeyVal_r));
					sscanf(++c, "%[^\n]", KeyVal_r); //获取'='右边的keyvalue
					a_trim(KeyVal_o, KeyVal_r);
					if (strcmp(KeyVal_o, KeyVal) != 0)
					{
						found = 2;
						memset(Keyline, 0, sizeof(Keyline));
						wcount += snprintf(wbuf + wcount, wlen - wcount, "%s\t=\t%s \n", KeyName, KeyVal);
						fread(wbuf + wcount, 1, wlen - wcount, fp);
					}
					else
					{ //值未改变，不写入
						found = 3;
					}
					break;
				}
				else
				{
					wcount += snprintf(wbuf + wcount, wlen - wcount, "%s", buf_i);
					continue;
				}
			}
		}
	}
	fclose(fp);
	if (found == 2)
	{
		fp = fopen(profile, "w+");
		fwrite(wbuf, 1, strlen(wbuf), fp);
		fclose(fp);
		free(wbuf);
		return 0;
	}
	else if (found == 3)
	{
		free(wbuf);
		return 0;
	}
	else
	{
		free(wbuf);
		return -1;
	}
}

/**更改配置文件配置参数,只修改已有的键值
 *@profile：文件完整路径名称
 *@KeyName：关键字名称
 *@KeyVal：要写入的建值
 *@return：结果返回0=OK,-1=ERROR
 */
int SetConfigFile_Old(char *profile, char *KeyName, char *KeyVal)
{
	char key_buf[32];
	char *buf_p;
	char buf_i[KEYVALLEN], buf_o[KEYVALLEN];
	char KeyVal_o[KEYVALLEN], KeyVal_r[KEYVALLEN];
	char Keyline[KEYVALLEN];
	FILE *fp;
	int found = 0; /*1=write Keyval 2=not write */
	if ((fp = fopen(profile, "r")) == NULL)
	{
		//文件打开失败
		perror("config file open error:");
		return (-1);
	}
	fseek(fp, 0, SEEK_END);
	int fsize = ftell(fp); //读取文件长度
	fseek(fp, 0, SEEK_SET);
	int wcount = 0, wlen = fsize + KEYVALLEN;
	char * wbuf = malloc(wlen);
	memset(wbuf, 0, wlen);
	while (!feof(fp) && fgets(buf_i, KEYVALLEN, fp) != NULL)
	{
		if (strlen(buf_i) >= (KEYVALLEN - 1) && buf_i[KEYVALLEN - 1] != '\n') //字符串长度过长,可能不包含‘\n’
		{
			wcount += snprintf(wbuf + wcount, wlen - wcount, "%s", buf_i);
			continue;
		}
		l_trim(buf_o, buf_i);
		if (strlen(buf_o) <= 0)
		{
			wcount += snprintf(wbuf + wcount, wlen - wcount, "%s", buf_i);
			continue;
		}
		buf_p = buf_o;
		if (buf_p[0] == '#')
		{
			wcount += snprintf(wbuf + wcount, wlen - wcount, "%s", buf_i);
			continue;
		}
		else
		{

			memset(key_buf, 0, sizeof(key_buf));
			memset(KeyVal_r, 0, sizeof(KeyVal_r));
			sscanf(buf_p, "%[^ |^\t] %[^\n]", key_buf, KeyVal_r); //获取' '左边的keyname
			if (strcmp(key_buf, KeyName) == 0)
			{
				a_trim(KeyVal_o, KeyVal_r);
				if (strcmp(KeyVal_o, KeyVal) != 0)
				{
					found = 1;
					memset(Keyline, 0, sizeof(Keyline));
					wcount += snprintf(wbuf + wcount, wlen - wcount, "%s %s\n", KeyName, KeyVal);
					fread(wbuf + wcount, 1, wlen - wcount, fp);
				}
				else
				{ //值未改变，不写入
					found = 2;
				}
				break;
			}
			else
			{
				wcount += snprintf(wbuf + wcount, wlen - wcount, "%s", buf_i);
				continue;
			}
		}
	}
	fclose(fp);
	if (found == 1)
	{
		fp = fopen(profile, "w+");
		fwrite(wbuf, 1, wcount, fp);
		fclose(fp);
		free(wbuf);
		return 0;
	}
	else if (found == 2)
	{
		free(wbuf);
		return 0;
	}
	else
	{
		free(wbuf);
		return -1;
	}
}

