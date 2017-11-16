// 
// 创建人： levy
// 创建时间：Mar 1, 2017
// 功能：main.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Mar 1, 2017                  levy          初版
// 
//
//#include <sqlite3.h>
//#include <stdio.h>
//#include <unistd.h>
//#include <signal.h>
//
//void vdgs_exit(void);
//void vdgs_sigquit(int sig);
//void vdgs_daemon();
//void vdgs_start_init();
//
//int main(int argc, char **argv)
//{
//	vdgs_daemon(); //设置进程到后台服务运行
//	RUN_LOG(NULL, "vdgs process start!"); //记录程序启动日志
//	atexit(&vdgs_exit); //注册异常退出时调用程序
//	signal(SIGQUIT, vdgs_sigquit); //注册收到quit信号后调用程序
//	vdgs_start_init(); //vdgs服务启动初始化程序
//	DEBUG_PRINTF("main process end\n");
//	struct timeval tv = { 5, 0 };
//	while (select(0, NULL, NULL, NULL, &tv) == 0)
//	{
//		tv.tv_sec = 5;
//		tv.tv_usec = 0;
//	}
//	pthread_exit(NULL);
//}
//
////程序运行异常结束调用
//void vdgs_exit(void)
//{
//	mysql_end();
//	mssql_uninit();
//	DWIN_UnInit();
//	//clear_messagequene();
//	ERR_LOG(NULL, "vdgs process fault end");
//	mylog_fini();
//	_exit(0);
//}
////收到quit信号调用结束程序
//void vdgs_sigquit(int sig)
//{
//	mysql_end();
//	mssql_uninit();
//	DWIN_UnInit();
//	//clear_messagequene();
//	RUN_LOG(NULL, "vdgs process end!");
//	mylog_fini();
//	_exit(0);
//}
//
////设置程序为后台服务程序
//void vdgs_daemon()
//{
//	char *pidfpname = getenv("PIDFILE");
//	if (pidfpname == NULL || *pidfpname == '\0')
//	{
//		printf("In debug mode\n");
//		return;
//	}
//	FILE * pidfp = fopen(pidfpname, "w+");
//	if (pidfp == NULL)
//	{
//		printf("can not open pidfile...");
//		exit(1);
//	}
//	if (daemon(0, 0) < 0)
//	{
//		perror("error daemon..");
//		exit(1);
//	}
//	fprintf(pidfp, "%d\n", (int) getpid());
//	fclose(pidfp);
//}
//
//void vdgs_start_init()
//{
//
//	DWIN_Init();	//触摸屏初始化配置
//
//}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include <bms_def.h>
#include <features.h>
#include "main.h"
#include <time.h>
cJSON * convert_object_to_json(Battery_main_def *bms);
void  ts_printbattrytab(char * buf,Battery_cell_def*cell);
Battery_cell_def cell;
//char *uint_array[256];
//char (*uint_arrayp)[256];
int main(void)
{
	struct tm tv;
	char sbuf[256];
	memset(&tv, 0, sizeof(struct tm));

	strptime("16-12-05 12:12","%y-%m-%d",&tv);
	strftime(sbuf, sizeof(sbuf), "%Y-%m-%d %H:%M", &tv);
	printf("do we have the s = %s",sbuf);
	//bms_init();
	//printf("main is over.\n");
//	char test[256]=" I am you.";
//	Battery_main_def batt={32,33,12};
//	memcpy(test,test+1,10);
    /* print the version */
    //printf("Version: %s\n", cJSON_Version());

    /* Now some samplecode for building objects concisely: */
    //create_objects();
//	cJSON* bms=convert_object_to_json(&batt);
//	char buf[1024];
//	ts_printbattrytab(buf,&cell);
//	printf("%s\n",test);
//	print_preallocated(bms);
//	cJSON_PrintPreallocated(bms,buf,sizeof(buf),0);
//	printf("%s",buf);
//	cJSON_Delete(bms);
//	cJSON * root=NULL;//cJSON_CreateObject();
//	int ss[3]={1,2,3};
//    root = cJSON_CreateIntArray(ss, 3);
//    print_preallocated(root);
//    cJSON_Delete(root);
    return 0;
}
