// 
// 创建人： wnet
// 创建时间：Aug 20, 2017
// 功能：mylog_base.c
// Copyright (c) 2016 wnet. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Aug 20, 2017                  wnet          初版
// 


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

const char *log_file_dir="/opt/wnet/log";
//const char *log_file_dir=".";
const char *base_name="wnet_log";
const int max_times=10;
const int max_size=1024*1024;

ssize_t get_write_file_size(const char *path)
{
	ssize_t size =-1;
	struct stat sbuf;
	if(access(path,W_OK)!=0)
	{
		return size;
	}else if(stat(path,&sbuf)<0)
	{
		return size;
	}else
	{
		size=sbuf.st_size;
	}
	return size;
}

void file_roll_back(const char *path)
{
	char new_file[128];
	char old_file[128];
	for(int i = max_times;i>1;i--)
	{
		snprintf(new_file,128,"%s%d",path,i);
		snprintf(old_file,128,"%s%d",path,i-1);
		if(access(old_file,F_OK)==0)
		{
			rename(old_file,new_file);
		}
	}
	if(access(path,F_OK)==0)
	{
		rename(path,old_file);
	}
}
FILE * open_log_file()
{
	FILE*fp=NULL;
	ssize_t size;
	char filename[128];
	if(log_file_dir[strlen(log_file_dir)-1]=='/')
	{
		snprintf(filename,128,"%s%s",log_file_dir,base_name);
	}else
	{
		snprintf(filename,128,"%s/%s",log_file_dir,base_name);
	}
	size = get_write_file_size(filename);
	if(size<0)
	{
		fp=fopen(filename,"w+");
	}else if(size < max_size)
	{
		fp=fopen(filename,"a+");
	}else if(size >= max_size)
	{
		file_roll_back(filename);
		fp = fopen(filename,"w+");
	}
	return fp;
}

const char *mylog_get_priority_name(int priority)
{
	const char * res=NULL;
	switch(priority)
	{
		case 1:
			res= "error";
			break;
		case 2:
			res = "warning";
			break;
		case 3:
			res = "info";
			break;
		case 4:
			res = "debug";
			break;
		default :
			res = NULL;
			break;
	}
	return res;
}

int mylog_write(const char * name,int priority,const char *time,const char *message)
{
	FILE *fp;
	fp = open_log_file();
	if(fp)
	{
		fprintf(fp,"%s %s [%s]:%s \n",time,name,mylog_get_priority_name(priority),message);
		fclose(fp);
		return 0;
	}
	else
	{
		perror("mylog_write");
		fclose(fp);
		return -1;
	}
}







