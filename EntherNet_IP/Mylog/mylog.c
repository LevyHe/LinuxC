// 
// 创建人： levy
// 创建时间：Jun 9, 2017
// 功能：mylog.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Jun 9, 2017                  levy          初版
// 

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

static inline struct tm* time_now()
{
	time_t timer = time(NULL); //获得当前时间
	return (localtime(&timer));
}
#define TIME_TO_STR(SBUF,SIZE,TM) strftime(SBUF,SIZE,"%Y-%m-%d %H:%M:%S",TM)

//void mylog_vlog(mylog_category_t * cat,int log_priority,const char* format,  va_list a_args)
//{
//	mylog_t *vlog = malloc(sizeof(mylog_t));
//	char tbuf[32];
//	struct timeval tmv;
//	strftime(tbuf, 32,"%Y-%m-%d %H:%M:%S", time_now());
//	gettimeofday(&tmv, NULL);
//	snprintf(vlog->v_time,32,"%s.%03d",tbuf,tmv.tv_usec/1000);
//	va_list va;
//	va_start(va, format);
//	vlog->message_len=vsnprintf(vlog->message,sizeof(vlog->message),format,va);
//	va_end(va);
//	cat->interface();
//}












