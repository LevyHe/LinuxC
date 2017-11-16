// 
// 创建人： levy
// 创建时间：Jun 6, 2016
// 功能：mylog.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期                                   负责人                           变更内容
// ───────────────────────────────────────────
// V0.01         Jun 6, 2016                  levy          初版
// 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include "mylog.h"
#include <errno.h>

#define  CRITICAL_SECTION   pthread_mutex_t
#define  _vsnprintf         vsnprintf
//Log{

#include <time.h>
#include <sys/timeb.h>
#include <stdarg.h>
char Errlogfile[] = ERR_LOG_FILE;
char Runlogfile[] = RUN_LOG_FILE;
static char logstr[MAXLINSIZE + 1];
char datestr[16];
char timestr[16];
char mss[4];
CRITICAL_SECTION cs_log = PTHREAD_MUTEX_INITIALIZER;
CRITICAL_SECTION cs_err = PTHREAD_MUTEX_INITIALIZER;
FILE *flog;

void Lock(CRITICAL_SECTION *l)
{
	pthread_mutex_lock(l);
}
void Unlock(CRITICAL_SECTION *l)
{
	pthread_mutex_unlock(l);
}

void LogV(const char *filename, const char *pszFmt, va_list argp)
{
	struct tm *now;
	struct timeb tb;
	char *filenamebk;
	if (NULL == pszFmt || 0 == pszFmt[0])
		return;
	_vsnprintf(logstr, MAXLINSIZE, pszFmt, argp);
	ftime(&tb);
	now = localtime(&tb.time);
	sprintf(datestr, "%04d-%02d-%02d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday);
	sprintf(timestr, "%02d:%02d:%02d", now->tm_hour, now->tm_min, now->tm_sec);
	sprintf(mss, "%03d", tb.millitm);
	DEBUG_PRINTF("%s %s.%s %s", datestr, timestr, mss, logstr);
	flog = fopen(filename, "a");
	if (NULL != flog)
	{
		fprintf(flog, "%s %s.%s %s", datestr, timestr, mss, logstr);
		if (ftell(flog) > MAXLOGSIZE)
		{
			fclose(flog);
			filenamebk = malloc(strlen(filename) + 10);
			strcpy(filenamebk, filename);
			strcat(filenamebk, "bk");
			remove(filenamebk);
			rename(filename, filenamebk);
		}
		else
		{
			fclose(flog);
		}
	}

}

void Log(char *filename, const char *pszFmt, ...)
{
	va_list argp;

	Lock(&cs_log);
	va_start(argp, pszFmt);
	LogV(filename, pszFmt, argp);
	va_end(argp);
	Unlock(&cs_log);
}

void Errthrew(const char *pszFmt, ...)
{
	va_list argp;
	Lock(&cs_err);
	va_start(argp, pszFmt);
	vfprintf(stderr, pszFmt, argp);
	va_end(argp);
	Unlock(&cs_err);
}

struct tm* time_now()
{
	time_t timer = time(NULL); //获得当前时间
	return (localtime(&timer));
}

void Resetstderrmode() //重设stderr缓存区，更改为缓存去慢输出
{
	setvbuf(stderr, NULL, _IOFBF, BUFSIZ);
}

void Log_init()
{
	pthread_mutex_init(&cs_log, NULL);
}

void log_uinit()
{
	pthread_mutex_destroy(&cs_log);
}
//Log}

int my_error(const char *fmt, ...)
{
	va_list args;
	int r;
	int err = errno;
	if (!getenv("MY_ERROR"))
		return 0;

	r = fprintf(stderr, "[ERROR:%s] ", strerror(err));
	va_start(args, fmt);
	r += vfprintf(stderr, fmt, args);
	va_end(args);
	r += fprintf(stderr, "\n");
	fflush(stderr);
	return r;
}

int my_debug(const char *fmt, ...)
{
	va_list args;
	int r;

	if (!getenv("MY_DEBUG"))
		return 0;

	r = fprintf(stdout, "[DEBUG] ");
	va_start(args, fmt);
	r += vfprintf(stdout, fmt, args);
	va_end(args);
	r += fprintf(stdout, "\n");
	return r;
}

