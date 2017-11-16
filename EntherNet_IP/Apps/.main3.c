////
//// 创建人： levy
//// 创建时间：May 25, 2017
//// 功能：main.c
//// Copyright (c) 2016 levy. All Rights Reserved.
//// Ver          变更日期        负责人                           变更内容
//// ──────────────────────────────────────────────────────────────────────────
//// V0.01         May 25, 2017                  levy          初版
////
//
//#include <stdlib.h>
//#include <stdio.h>
//#include <unistd.h>
//#include <time.h>
//#include <sys/time.h>
//#include <pthread.h>
//#include <string.h>
//#include <errno.h>
//#include "mysql_pool.h"
//#include <stdint.h>
//#include "mysql_assist.h"
//extern unsigned int res_count;
////const char *test_sql = "SELECT ID,Name,Value,RecordTime FROM HY_VDGS_ParamHistory where ID =0";
//const char *test_sql = "select * from HY_VDGS_LeadCmd;";
//extern unsigned int query_times ;
//extern  TagStruct *_pbb_taglist;
//extern  CIP * _pbb_client;
//
////void * test_thread(void *arp)
////{
////	int flag=0;
////	int count=0;
////
////	MYSQL_RES * resptr;
////	for(int i=0;i<10;i++)
////	{
////		resptr=mysql_execute_query(test_sql,strlen(test_sql),&flag);
////		if(resptr !=NULL )
////		{
////			printf("resptr is not null\n");
////			mysql_free_result(resptr);
////		}
////	}
//////	{
//////		resptr=mysql_execute_query(test_sql,strlen(test_sql),&flag);
//////		if(resptr)
//////		{
//////			count++;
//////			mysql_free_result(resptr);
//////		}
//////		else
//////		{
//////			perror("error");
//////		}
//////	}
////	//printf("I has finsh %d query.\n",count);
////	pthread_exit(NULL);
////}
//
//TagStruct *testlist=NULL;
//int cip_taglist_sync();
//int pbb_client_init();
//#define thread_nums 10000
//char* get_rmem(pid_t p);
////int  main(void )
////{
////
////	TagStruct * tag;
//////	int sql_size = 128*1000;
//////	char *sql_buf=malloc(sql_size);
////	int num=0;
////	int mem_size;
////	struct timeval tv;
////	 pid_t pid= getpid();
////	 printf("current pid:%d,%s \n",pid ,get_rmem(pid));
////	mysql_pool_init();
////	pbb_client_init();
//////	pthread_t th[thread_nums];
////	struct timeval tnow,tnow1,tnow2;
////	gettimeofday(&tnow1, NULL);
////	//printf("start time tv_sec = %ld,tv_usec = %ld \n",tnow1.tv_sec,tnow1.tv_usec);
//////	for(int i=0;i<thread_nums;i++)
//////	{
//////		pthread_create(&th[i],NULL,test_thread,NULL);
//////	}
//////	for(int i=0;i<thread_nums;i++)
//////	{
//////		pthread_join(th[i],NULL);
//////	}
//////	for(int i=0 ; i<1000;i++)
//////	{
//////		num += snprintf(sql_buf+num,sql_size-num,"insert into test(id,value) values(0,%d);",i);
//////	}
//////	mysql_commit_query(sql_buf,num);
////	//mem_size=get_rmem(pid);
////	printf("current pid:%d,%s \n",pid ,get_rmem(pid));
////	while(num<100)
////	{
////
////		cip_taglist_sync();
////		tv.tv_sec=0;
////		tv.tv_usec=500000;
////		select(0,NULL,NULL,NULL,&tv);
////		num++;
////		//printf("current pid:%d,%s \n",pid ,get_rmem(pid));
////	}
////	num=0;
////	printf("***********_pbb_taglist*************\n");
////	for(tag = _pbb_taglist;tag;tag = tag->next)
////	{
////		printf("%d.tagname=%s;\n",num++,tag->tag);
////	}
////	num=0;
////	printf("***********_pbb_client->MutileTagList*************\n");
////	for(tag = _pbb_client->MutileTagList;tag;tag = tag->next)
////	{
////		printf("%d.tagname=%s;\n",num++,tag->tag);
////	}
////	ClearTagList(&_pbb_taglist);
////	printf("current pid:%d,%s \n",pid ,get_rmem(pid));
//////	pbb_client_init();
////	gettimeofday(&tnow2, NULL);
////	timersub(&tnow2,&tnow1,&tnow);
////	printf("Max keep connections = %d,res_count=%d,total query times=%d,use %ld us.\n",MAX_KEPP_CONNECTIONS,res_count,query_times,tnow.tv_sec*1000000+tnow.tv_usec);
////	//printf("total time tv_sec = %ld.%3ld s \n",);
////	destory_mysql_pool();
////	tv.tv_sec=0;
////	tv.tv_usec=500000;
////	select(0,NULL,NULL,NULL,&tv);
////	printf("current pid:%d,%s \n",pid ,get_rmem(pid));
////	pthread_exit(NULL);
////}
//const char *test_sql = "select * from HY_VDGS_LeadCmd;";
//extern unsigned int query_times ;
//extern  TagStruct *_pbb_taglist;
//extern  CIP * _pbb_client;
//
//
//
//TagStruct *testlist=NULL;
//int cip_taglist_sync();
//int pbb_client_init();
//#define thread_nums 10000
//char* get_rmem(pid_t p);
//int  main(void )
//{
//
//	TagStruct * tag;
//
//	int num=0;
//	struct timeval tv;
//	 pid_t pid= getpid();
//	 printf("current pid:%d,%s \n",pid ,get_rmem(pid));
//	mysql_pool_init();
//	pbb_client_init();
////	pthread_t th[thread_nums];
//	struct timeval tnow,tnow1,tnow2;
//	gettimeofday(&tnow1, NULL);
//
//	printf("current pid:%d,%s \n",pid ,get_rmem(pid));
//	while(num<100)
//	{
//
//		//pbb_taglist_sync();
//		tv.tv_sec=0;
//		tv.tv_usec=500000;
//		select(0,NULL,NULL,NULL,&tv);
//		num++;
//	}
//
//	gettimeofday(&tnow2, NULL);
//	timersub(&tnow2,&tnow1,&tnow);
//	printf("Max keep connections = %d,res_count=%d,total query times=%d,use %ld us.\n",MAX_KEPP_CONNECTIONS,res_count,query_times,tnow.tv_sec*1000000+tnow.tv_usec);
//	destory_mysql_pool();
//	tv.tv_sec=0;
//	tv.tv_usec=500000;
//	select(0,NULL,NULL,NULL,&tv);
//	printf("current pid:%d,%s \n",pid ,get_rmem(pid));
//	pthread_exit(NULL);
//}
//
////程序运行异常结束调用
//void vdgs_exit(void)
//{
//	pbb_client_close();
//	destory_mysql_pool();
//	_exit(0);
//}
