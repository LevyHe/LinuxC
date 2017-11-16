// 
// 创建人： levy
// 创建时间：Jun 7, 2017
// 功能：pbbms.h
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Jun 7, 2017                  levy          初版
// 


#ifndef PBBMS_H_
#define PBBMS_H_

#define SignleList_Push(ptr,l,type) do {\
type * v;\
	v=*(ptr);\
	(l)->next=NULL;\
	if(v==NULL){\
	*(ptr)=(l);	\
	}else{\
		while(v->next){\
		v=v->next;}\
		v->next=(l);\
	}}while(0)
#define SignleList_Clear(ptr,type)do {\
	type * v;\
	type * n;\
	for(v=*(ptr);v;v=n){\
		n=v->next;\
		free(v);\
	}\
	*(ptr)=NULL;\
	}while(0)


typedef struct RecordTagStruct
{
	struct RecordTagStruct * next;
	TagStruct tag;
	TagStruct * mtag;	//用于请求的标签
	char tagname[32];
	char update_time[32];
}RecordTagStruct;



int pbb_client_init();
void pbb_client_close();
int pbb_cip_taglist_sync();
void pbb_tag_update_value(void * arg);
void pbb_tag_write_fpt(void *arg);
void pbb_test();

#endif /* PBBMS_H_ */
