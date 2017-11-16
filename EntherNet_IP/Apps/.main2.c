// 
// 创建人： levy
// 创建时间：May 25, 2017
// 功能：main.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         May 25, 2017                  levy          初版
// 

//#include <stdlib.h>
//#include <stdio.h>
//#include <unistd.h>
//#include <time.h>
//#include <sys/time.h>
//#include <pthread.h>
//#include <string.h>
//#include <errno.h>
//
//pthread_mutex_t test_mutex=PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t test_cond=PTHREAD_COND_INITIALIZER;
//
////pthread_t th;
////pthread_t th1;
////pthread_t th2;
//int test_count;
//static void set_abswaittime(struct timespec*outtime, int ms)
//{
//	long sec ;
//	long usec ;
//	struct timeval tnow;
//	gettimeofday(&tnow, NULL);
//	usec = tnow.tv_usec + ms*1000;
//	sec =  tnow.tv_sec+usec/1000000;
//	outtime->tv_nsec=(usec%1000000)*1000;
//	outtime->tv_sec=sec;
//
//}
//
//void * test_thread1(void *arp)
//{
//	int s=(int)arp;
//	printf("I am thread %ld ,I will sleep %d s .\n",pthread_self(),s);
//	sleep(s);
//	pthread_mutex_lock(&test_mutex);
//
//	pthread_cond_wait(&test_cond,&test_mutex);
//	test_count++;
//	printf("my thread id is %ld ,testcount=%d \n",pthread_self(),test_count);
//	pthread_mutex_unlock(&test_mutex);
//	pthread_exit(NULL);
//}


//void * test_thread2(void *arp)
//{
//	int res;
//	if(pthread_detach(pthread_self()))
//	{
//		pthread_exit(NULL);
//	}
//	sleep(1);
//	pthread_mutex_lock(&test_mutex);
//	res=pthread_cond_wait(&test_cond,&test_mutex);
//	pthread_mutex_unlock(&test_mutex);
//	res=pthread_cond_destroy(&test_cond);
//	res=pthread_cond_wait(&test_cond,&test_mutex);
//	printf("test_thread22:%d,%s\n",res,strerror(res));
//	pthread_exit(NULL);
//}


//int  main(void )
//{
//	int res;
//	pthread_t th[3];
//	test_count=0;
//	for(int i=0;i<3;i++)
//	{
//		pthread_create(&th[i],NULL,test_thread1,1);
//	}
//
//	for(int i=0;i<3;i++)
//	{
//		sleep(2);
//		res=pthread_cond_signal(&test_cond);
//		//printf("what happens :%d,%s\n",res,strerror(res));
//	}
//	for(int i=0;i<3;i++)
//	{
//		pthread_join(th[i],NULL);
//	}
//	//int res=0;
//	//struct timespec tp;
//	//struct timeval tnow;
//	//res=pthread_mutex_init(&test_mutex,NULL);
//	//printf("main1:%d,%s\n",res,strerror(errno));
//	//res=pthread_mutex_lock(&test_mutex);
//	//res=pthread_mutex_unlock(&test_mutex);
//	//perror("as");
//	//printf("1:%d,%s\n",res,strerror(errno));
//	//res=pthread_mutex_lock(&test_mutex);
//	//pthread_create(&th,NULL,test_thread,NULL);
//	//pthread_create(&th1,NULL,test_thread2,NULL);
//	//pthread_create(&th2,NULL,test_thread2,NULL);
//	//set_abswaittime(&tp,1000);
//	//res=pthread_mutex_timedlock(&test_mutex,&tp);
//
//	pthread_exit(NULL);
//}
