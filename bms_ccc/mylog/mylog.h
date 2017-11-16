// 
// 创建人： levy
// 创建时间：2016-6-5
// 功能：mysql.h
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期                                   负责人                           变更内容
// ───────────────────────────────────────────
// V0.01         2016-6-5                  levy          初版
// 

#ifndef MYLOG_H_
#define MYLOG_H_

#include <sys/time.h>
#include <mylib.h>
#include <string.h>
#include <errno.h>

#define MAXLOGSIZE 1024000
#define MAXLINSIZE 16000
#define FILEINFOLEN 200

#define __DEBUG_MODE 1
#define __LOG4C__ 0

#define ERR_LOG_FILE "Err.log"
#define RUN_LOG_FILE "Run.log"

extern void Log(char *filename, const char *pszFmt, ...);
extern void Log_init();
extern void log_uinit();
extern void Errthrew(const char *pszFmt, ...);
extern char Errlogfile[];
extern char Runlogfile[];
extern void Resetstderrmode();
extern struct tm* time_now();

extern int my_error(const char *fmt, ...);
extern int my_debug(const char *fmt, ...);

#if __LOG4C__
extern void err_message(char* file, const char* func, int line, const char* a_format, ...);
extern void run_message(char* file, const char* func, int line, const char* a_format, ...);
extern int mylog_init();
extern int mylog_fini();
extern void Errthrew_log4c(const char *pszFmt, ...);


#define ERR_LOG(name,pszFmt,args...) err_message(__FILE__,__FUNCTION__, __LINE__," Desc["pszFmt"]\n",## args)
#define RUN_LOG(name,pszFmt,args...) run_message(__FILE__,__FUNCTION__, __LINE__," Desc["pszFmt"]\n",## args)
#define DEBUG_LOG(name,pszFmt,args...) debug_message(__FILE__,__FUNCTION__, __LINE__," Desc["pszFmt"]\n",## args)

#define ERR_THREW(pszFmt,args...) Errthrew_log4c("[ERROR:%s] [%s] [%s()] [LINE:%d] Desc["pszFmt"]\n",strerror(errno), __FILE__,__FUNCTION__, __LINE__,## args)

#define ERR_MSG(pszFmt,args...) my_error("[%s]-[%s]-[LINE:%d]:"pszFmt"\n", __FILE__,__FUNCTION__, __LINE__,## args)
#define DEBUG_MSG(pszFmt,args...) my_debug("[%s]-[%s]-[LINE:%d]:"pszFmt"\n", __FILE__,__FUNCTION__, __LINE__,## args)

#else

#define ERR_MSG(pszFmt,args...) my_error("[%s]-[%s]-[LINE:%d]:"pszFmt"\n", __FILE__,__FUNCTION__, __LINE__,## args)
#define DEBUG_MSG(pszFmt,args...) my_debug("[%s]-[%s]-[LINE:%d]:"pszFmt"\n", __FILE__,__FUNCTION__, __LINE__,## args)


#define ERR_LOG(name,pszFmt,args...) Log(Errlogfile,"[ERROR] [%s]-[%s] [%s] [LINE:%d]:" pszFmt "\n",name, __FILE__,__FUNCTION__, __LINE__,## args)

//#define ERR_LOG(name,pszFmt,args...) Log(Errlogfile,ERR_FORMAT(pszFmt),name, __FILE__,__FUNCTION__, __LINE__,## args)
#define RUN_LOG(name,pszFmt,args...) Log(Runlogfile,"[LOG] [%s]-[%s] [%s] [LINE:%d]:"#pszFmt"\n",name, __FILE__,__FUNCTION__, __LINE__,## args)

#define ERR_THREW(pszFmt,args...) Errthrew("[ERROR] [%s] [%s] [LINE:%d]:"#pszFmt"\n", __FILE__,__FUNCTION__, __LINE__,## args)
#endif
#define TIME_TO_STR(SBUF,SIZE,TM) strftime(SBUF,SIZE,"%Y-%m-%d %H:%M:%S",TM)

#define STR_TO_TIME(STR,TM) strptime(STR, "%Y-%m-%d %H:%M:%S", TM)

#if __DEBUG_MODE
#define DEBUG_PRINTF printf
#else
#define DEBUG_PRINTF(pszFmt,args...)
#endif

#endif /* MYLOG_H_ */
