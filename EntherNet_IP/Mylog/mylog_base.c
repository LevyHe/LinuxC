// 
// 创建人： levy
// 创建时间：Jun 9, 2017
// 功能：mylog_base.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Jun 9, 2017                  levy          初版
// 


#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

char * log_file_dir;

char * base_name;

int max_times;

int max_size;


ssize_t get_write_file_size(const char * path)
{
	ssize_t size =-1;
	struct stat sbuf;
	if(access(path,W_OK)!=0)
	{
		return size;
	}else if(stat(path,&sbuf)<0)
	{
		return size;
	}
	else
	{
		size=sbuf.st_size;
	}
	return size;
}

void file_roll_back(const char * path )
{
	char new_flie[128];
	char old_flie[128];
	for (int i = max_times;i>0;i--)
	{
		snprintf(new_flie,128,"%s%d",path,i);
		snprintf(old_flie,128,"%s%d",path,i-1);
		if(access(old_flie,F_OK)==0)
		{
			rename(old_flie,new_flie);
		}
	}
	if(access(path,F_OK)==0)
	{
		rename(path,old_flie);
	}
}

FILE * open_log_file()
{
	FILE* fp=NULL;
	ssize_t size;
	char filename[128];

	snprintf(filename,128,"%s/%s");
	size = get_write_file_size(filename);
	if(size<0)
	{
		fp =fopen(filename,"w+");
	}else if(size < max_size)
	{
		fp =fopen(filename,"a+");
	}else if(size > max_size)
	{
		file_roll_back(filename);
		fp =fopen(filename,"w+");
	}
	return fp;
}

int mylog_write(const char * name,const char *priority,const char *time,const char *message)
{
	FILE * fp;
	fp = open_log_file();
	if(fp)
	{
		fprintf(fp,"%s %s [%s]:%s \n",time,name,priority,message);
		//fflush(fp);
		fclose(fp);
		return 0;
	}
	else
	{
		perror("mylog_write");
		return -1;
	}
}

//获取不同log优先级的名称
const char * mylog_get_priority_name(int priority)
{
	switch(priority)
	{
		case 1:
			return "error";
			break;
		case 2:
			return "warnning";
			break;
		case 3:
			return "info";
			break;
		case 4:
			return "debug";
			break;
		default:
			return NULL;
			break;
	}
}
//
//int mylog_vlog_appender(mylog_category_t * cat,int log_priority,const char* format,  va_list a_args)
//{
//
//}





