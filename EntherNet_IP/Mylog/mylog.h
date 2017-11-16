// 
// 创建人： levy
// 创建时间：Jun 9, 2017
// 功能：mylog.h
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Jun 9, 2017                  levy          初版
// 


#ifndef MYLOG_H_
#define MYLOG_H_


#define IS_MYLOG_ERR_PRINTF	1
#define IS_MYLOG_PRINTF 1
#define MYLOG_MAX_MESSAGE_SIZE 256

#define MYLOG_PRIORITY_ERROR 	1
#define MYLOG_PRIORITY_WARN		2
#define MYLOG_PRIORITY_INFO		3
#define MYLOG_PRIORITY_DEBUG	4

typedef struct mylog_t
{
	struct mylog_t *next;
	int log_priority;	//消息等级
	mylog_category_t * cat;
	char message[MYLOG_MAX_MESSAGE_SIZE];
	int message_len;
	char v_time[32];
}mylog_t;

//类别存储接口名称
typedef struct
{
	const char cat_name[128];
	mylog_t * msg_list;
	void (*interface)( void * mylog_cat,mylog_t*log_msg);
}mylog_category_t;



#if IS_MYLOG_PRINTF
#define MYLOG_PRINTF printf
#else
#define MYLOG_PRINTF(pszFmt,args...)
#endif

#if IS_MYLOG_ERR_PRINTF
#define MYLOG_ERR_PRINTF printf
#else
#define MYLOG_ERR_PRINTF(pszFmt,args...)
#endif


#endif /* MYLOG_H_ */
