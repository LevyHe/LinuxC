// 
// 创建人： wnet
// 创建时间：Aug 20, 2017
// 功能：mylog.h
// Copyright (c) 2016 wnet. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Aug 20, 2017                  wnet          初版
// 


#ifndef MYLOG_H_
#define MYLOG_H_
#include <errno.h>
#include <string.h>

#define IS_DEBUG_LOG 0
#define IS_MYLOG_ERR_PRINTF 1
#define IS_MYLOG_PRINTF 1
#define MYLOG_MAX_MESSAGE_SIZE 1024

#define MYLOG_PRIORITY_ERROR 1
#define MYLOG_PRIORITY_WARN 2
#define MYLOG_PRIORITY_INFO 3
#define MYLOG_PRIORITY_DEBUG 4


typedef int (*interface_def)(const char * name,int priority,const char *time,const char *message);
int mylog(const char *cat_name,int priority,const char *msgFmt,...);
void mylog_init();
typedef struct mylog_category_t
{
	struct mylog_category_t *next;
	struct mylog_category_t *prev;
	char cat_name[128];
	char message[MYLOG_MAX_MESSAGE_SIZE];
	int log_priority;//
	char v_time[32];
	interface_def v_log;
}mylog_category_t;

#if IS_MYLOG_PRINTF
#define MYLOG_PRINTF printf
#else
#define MYLOG_PRINTF(pszFmt,arg,...)
#endif

#if IS_MYLOG_PRINTF
#define MYLOG_ERR_PRINTF printf
#else
#define MYLOG_ERR_PRINTF(pszFmt,arg,...)
#endif

#define ERR_LOG(pszFmt,args...) mylog("wnet",MYLOG_PRIORITY_ERROR,"at %s-%s-%d : "pszFmt,__FILE__,__FUNCTION__,__LINE__,##args)
#define RUN_LOG(pszFmt,args...) mylog("wnet",MYLOG_PRIORITY_INFO,pszFmt,##args)
#if IS_DEBUG_LOG
#define DEBUG_LOG(pszFmt,args...) mylog("wnet",MYLOG_PRIORITY_DEBUG,"at %s-%s-%d : "pszFmt,__FILE__,__FUNCTION__,__LINE__,##args)
#else
#define DEBUG_LOG(pszFmt,args...)
#endif



#endif /* MYLOG_H_ */
