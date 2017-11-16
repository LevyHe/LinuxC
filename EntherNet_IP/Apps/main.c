// 
// 创建人： levy
// 创建时间：May 25, 2017
// 功能：main.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         May 25, 2017                  levy          初版
// 

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include "mysql_pool.h"
#include <stdint.h>
#include "mysql_assist.h"
#include "pbbms.h"
#include <signal.h>
void main_exit(void);
void main_sigquit(int sig);
struct timeval tnow,tnow1,tnow2;
int  main(void )
{

	struct timeval tv;
	atexit(&main_exit); //注册异常退出时调用程序
	signal(SIGQUIT, main_sigquit); //注册收到quit信号后调用程序
	mysql_pool_init();
	pbb_client_init();
	int num=0;
	while(1)
	{
		pbb_cip_taglist_sync();
		tv.tv_sec=1;
		tv.tv_usec=0;
		select(0,NULL,NULL,NULL,&tv);
		if(num++%5==0)
		{
			//gettimeofday(&tnow1, NULL);
			//pbb_test();

		}

	}
	destory_mysql_pool();
	pthread_exit(NULL);
}

void main_sigquit(int sig)
{
	main_exit();
}

//程序运行异常结束调用
void main_exit(void)
{
	pbb_client_close();
	destory_mysql_pool();
	_exit(0);
}
