// 
// 创建人： wnet
// 创建时间：Aug 20, 2017
// 功能：mylog.c
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
//#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <stdarg.h>
#include "mylog.h"

static pthread_mutex_t log_lock=PTHREAD_MUTEX_INITIALIZER;
static mylog_category_t *cat_list=NULL;
extern int mylog_write(const char * name,int priority,const char *time,const char *message);

static inline struct tm* time_now()
{
	time_t timer = time(NULL);
	return (localtime(&timer));
}

#define TIME_TO_STR(sbuf,size,tm) strftime(sbuf,size,"%Y-%m-%d %H:%M:%S",tm)

mylog_category_t *get_mylog_cat(const char* cat_name)
{
	mylog_category_t *v=NULL;
	for(v = cat_list;v;v=v->next)
	{
		if(strcmp(v->cat_name,cat_name)==0)
		{
			break;
		}
	}
	return v;
}


int mylog_v(mylog_category_t *log)
{
	return log->v_log(log->cat_name,log->log_priority,log->v_time,log->message);
}

int mylog(const char *cat_name,int priority,const char *msgFmt,...)
{
	int res=0;
	pthread_mutex_lock(&log_lock);
	mylog_category_t *vlog=get_mylog_cat(cat_name);
	if(vlog)
	{
		va_list argp;
		va_start(argp,msgFmt);
		vsnprintf(vlog->message,sizeof(vlog->message),msgFmt,argp);
		va_end(argp);
		vlog->log_priority=priority;
		strncpy(vlog->cat_name,cat_name,sizeof(vlog->cat_name));
		TIME_TO_STR(vlog->v_time,sizeof(vlog->v_time),time_now());
		res=mylog_v(vlog);
	}else
	{
		res=-1;
	}
	pthread_mutex_unlock(&log_lock);
	return res;
}

void mylog_add_cat(const char* cat_name,interface_def pfun)
{
	mylog_category_t* v;
	mylog_category_t*l=calloc(1,sizeof(mylog_category_t));
	l->v_log=pfun;
	strncpy(l->cat_name,cat_name,sizeof(l->cat_name));
	v=cat_list;
	if(v==NULL)
	{
		l->prev=NULL;
		cat_list=l;
	}else
	{
		while(v->next)
		{
			v=v->next;
			l->prev=v;
			v->next=l;
		}
	}
}

void mylog_init()
{
	mylog_add_cat("wnet",mylog_write);
}







