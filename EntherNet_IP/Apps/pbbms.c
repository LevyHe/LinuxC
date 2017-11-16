// 
// 创建人： levy
// 创建时间：Jun 7, 2017
// 功能：pbbms.c
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Jun 7, 2017                  levy          初版
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
#include <ctype.h>
#include "mysql_assist.h"

//数据库中的标签列表，与客户端中的标签列表可能不同
static TagStruct *_pbb_taglist=NULL;
static RecordTagStruct * _pbb_write_taglist=NULL;
static TagStruct * _pbb_signle_req_tag_list=NULL;
static pthread_mutex_t _pbb_write_tag_mutex=PTHREAD_MUTEX_INITIALIZER;
static pthread_t _pbb_thread_h;

static CIP * _pbb_client=NULL;
static void  _pbb_tag_value_update();
static void _pbb_write_tag_history();
static void *_pbb_get_write_taglist_thread(void*arg);
static void set_abswaittime(struct timespec*outtime, int ms)
{
	long sec ;
	long usec ;
	struct timeval tnow;
	gettimeofday(&tnow, NULL);
	usec = tnow.tv_usec + ms*1000;
	sec =  tnow.tv_sec+usec/1000000;
	outtime->tv_nsec=(usec%1000000)*1000;
	outtime->tv_sec=sec;
}


/*从字符串中取出纯数字，不包含任何其它字符
 * @buf 待转换的字符串
 * @digit 返回的数字
 * @return 0 包含其它数字，1 纯数字的字符串
 * */
static int is_get_digit(const char * buf,int * digit)
{
	for(const char * p = buf;*p!='\0';p++)
	{
		if(!isdigit(*p))
		{
			return 0;
		}
	}
	*digit=atoi(buf);
	return 1;
}
/*从带bit位的整形标签中，获取真正有效的标签名
 * @return -1 标签名没有转换，>=0 标签的bit位地址
 * */
int get_real_tag_name(char * out,const char *in)
{
	char *c_dot;
	const char * start  = in;
	int num;
	c_dot = strrchr(in , '.');
	if(c_dot&&is_get_digit(c_dot+1,&num))
	{
		while(start!=c_dot)
		{
			*out++=*start++;
		}
		*out=0;
		return num;
	}
	else
	{
		strcpy(out,in);
		return -1;
	}
}

/*检查taglist是否具有realname像类似的(除去bit位地址后，标签名相同)并且rw_flag一致的标签
 * @return 1 Yes，0 No
 * */
int is_realtagname_in_taglist(TagStruct**taglist,const char *realname,int rw_flag)
{
	TagStruct * tag ;
	char pbuf[128];
	for(tag = *taglist;tag;tag=tag->next)
	{
		get_real_tag_name(pbuf,tag->tag);
		if(tag->rw_flag == rw_flag && strncasecmp(pbuf,realname,128)==0)
		{
			break;
		}
	}
	return tag?1:0;
}

int pbb_client_init()
{
	char dest_ip[32];
	int dest_port=0;
	int res;
	res=mysql_get_tab_value("hy_pbbms_device_info","dest_ip",dest_ip,sizeof(dest_ip));
	if(res!=0)
	{
		return res;
	}
	res= mysql_get_tab_value("hy_pbbms_device_info","dest_port",&dest_port,sizeof(dest_port));
	if(res!=0)
	{
		return res;
	}
	if(dest_port==0)dest_port=44818;
	_pbb_client=cip_CreatNewClient(dest_ip,(uint16_t)dest_port);
	if(_pbb_client)
	{
		CIP_MutileFunctionBind(_pbb_client,pbb_tag_update_value);
		CIP_SignleFunctionBind(_pbb_client,pbb_tag_write_fpt);
		pbb_cip_taglist_sync();
		cip_ThreadStart(_pbb_client);
		pthread_create(&_pbb_thread_h, NULL, _pbb_get_write_taglist_thread, _pbb_client);
		return 0;
	}
	return -4;
}

/*同步数据库、PBB、客户端的标签名列表*/
int pbb_cip_taglist_sync()
{
	int res,rw_flag;
	TagStruct *list=NULL;
	TagStruct *tag;
	TagStruct * new_tag;
	char pbuf[128];
	res=cip_get_tag_list(&list);
	//同步list中新的标签到_pbb_taglist
	for(tag = list;tag;tag = tag->next)
	{
		if(GetTagFromListByTag(&_pbb_taglist,tag)==NULL)
		{
			new_tag=CreatTagStruct(tag->tag,tag->rw_flag);
			TagStructPush(&_pbb_taglist,new_tag);
			get_real_tag_name(pbuf,tag->tag);
			if(GetTagFromMutileTagList(_pbb_client,pbuf,tag->rw_flag)==NULL)
			{
				AddMutileReadTagRequst(_pbb_client,pbuf);
			}
		}
	}
	//删除_pbb_taglist中多余的标签
	for(tag = _pbb_taglist;tag;tag=new_tag )
	{
		new_tag = tag->next;
		if(GetTagFromListByTag(&list,tag)==NULL)
		{
			get_real_tag_name(pbuf,tag->tag);
			rw_flag = tag->rw_flag;
			RemoveTagFromTaglistByTag(&_pbb_taglist,tag);
			if(is_realtagname_in_taglist(&_pbb_taglist,pbuf,rw_flag)==0)
			{
				RemoveMutileTag(_pbb_client,pbuf,rw_flag);
			}
		}
	}
	ClearTagList(&list);
	return res;
}

void pbb_tag_update_value(void * arg)
{
	_pbb_tag_value_update();
}

void pbb_tag_write_fpt(void *arg)
{
//	CIP * cip = (CIP*)arg;
//	TagStruct *tag=cip->SignleRepTag;
//	if(tag)
//	printf("tag name is : %s errstate=%d\n",tag->tag,tag->error);
//	gettimeofday(&tnow2, NULL);
//	timersub(&tnow2,&tnow1,&tnow);
//	printf("requst one cycle use time=%ld ms.\n",(tnow.tv_sec*1000000+tnow.tv_usec)/1000);
//	ClearTagList(&cip->SignleRepTag);
//	pthread_mutex_unlock(&_pbb_write_tag_mutex);
	_pbb_write_tag_history();
}

/*pbb_client_close 程序结束调用*/
void pbb_client_close()
{
	pthread_cancel(_pbb_thread_h);
	cip_ThreadStop(_pbb_client);
	cip_ClientDisponse(_pbb_client);
	ClearTagList(&_pbb_signle_req_tag_list);
	SignleList_Clear(&_pbb_write_taglist,RecordTagStruct);
	ClearTagList(&_pbb_taglist);
}

void pbb_test()
{
	gettimeofday(&tnow1, NULL);
	int res=AddSignleReadTagRequst(_pbb_client,"READ[0]");
	printf("AddMutileReadTagRequst is %d\n",res);
}

static void  _pbb_tag_value_update()
{
	TagStruct *tag;
	TagStruct *vtag;
	TagStruct *list=NULL;
	TagStruct * new_tag;
	char pbuf[128];
	int num;
	for(tag=_pbb_taglist;tag;tag=tag->next)
	{
		num=get_real_tag_name(pbuf,tag->tag);
		vtag=GetTagFromMutileTagList(_pbb_client,pbuf,tag->rw_flag);
		if(vtag)
		{

			if(vtag->error==0)
			{
				if(num<0)//不包含bit位地址的标签
				{
					if(tag->type!=vtag->type || CIPValueCmp(&tag->value,&vtag->value,vtag->type)!=0)
					{
						tag->type = vtag->type;
						tag->error = 0;
						CipValueCopy(&tag->value,&vtag->value,vtag->type);
						new_tag=CloneTagStruct(tag);
						TagStructPush(&list,new_tag);
					}
				}
				else//包含bit位地址的标签
				{
					int val=get_cip_bit_value(&vtag->value,vtag->type,num);
					if(val<0)
					{
						tag->error = TAG_ERROR_BIT_ERROR;
						new_tag=CloneTagStruct(tag);
						TagStructPush(&list,new_tag);
					}else if(tag->type!=CIP_TYPE_BOOL || tag->value.bool!=val)
					{
						tag->type=CIP_TYPE_BOOL;
						tag->error = 0;
						tag->value.bool=val;
						new_tag=CloneTagStruct(tag);
						TagStructPush(&list,new_tag);
					}
				}
			}
			else
			{
				tag->error = vtag->error;
				new_tag=CloneTagStruct(tag);
				TagStructPush(&list,new_tag);
			}
		}
	}
	if(list)
	{
		cip_update_tag_value(list);
		ClearTagList(&list);
	}
}

static void _pbb_write_tag_history()
{

	TagStruct *tag;
	TagStruct *ptag;
	RecordTagStruct * rtag;
	for(tag = _pbb_signle_req_tag_list;tag;tag = tag->next)
	{
		ptag =GetTagFromListByTag(&_pbb_client->SignleRepTag,tag);
		if(ptag == NULL)
		{
			tag->error=TAG_ERROR_UNKNOWN;
		}else if(ptag->error!=0)
		{
			tag->error=ptag->error;
		}
	}
	for(rtag = _pbb_write_taglist;rtag;rtag=rtag->next)
	{
		if(rtag->tag.error ==0)
		{
			if(rtag->mtag ==NULL)
			{
				rtag->tag.error = TAG_ERROR_UNKNOWN;
			}else
			{
				rtag->tag.error=rtag->mtag->error;
			}
		}
		DEBUG_PRINTF("has write tag %s ,error = %d\n",rtag->tagname,rtag->tag.error);
	}
	cip_update_write_history(_pbb_write_taglist);
	SignleList_Clear(&_pbb_write_taglist,RecordTagStruct);
	ClearTagList(&_pbb_signle_req_tag_list);
	pthread_mutex_unlock(&_pbb_write_tag_mutex);
}

static void *_pbb_get_write_taglist_thread(void*arg)
{
	if(pthread_detach(pthread_self()))
	{
		pthread_exit(NULL);
	}
	if( pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL)!=0)
	{
		pthread_exit(NULL);
	}
	int res;
	struct timeval tv;
	struct timespec tp;
	RecordTagStruct * list = NULL;
	RecordTagStruct *tag;
	TagStruct * mtag;
	TagStruct * read_tag;
	TagStruct * new_tag;
	char pbuf[128];
	int num;
	while(1)
	{
		tv.tv_sec=0;
		tv.tv_usec=500000;
		select(0,NULL,NULL,NULL,&tv);
		list=NULL;
		res=cip_get_write_taglist(&list);
		if(res!=0)
		{
			DEBUG_PRINTF("cip_get_write_taglist error is [%d] %s\n",errno,strerror(errno));
		}
		set_abswaittime(&tp,2000);
		res=pthread_mutex_timedlock(&_pbb_write_tag_mutex,&tp);
		if(res == ETIMEDOUT)
		{
			for(tag = _pbb_write_taglist;tag;tag=tag->next)
			{
				tag->tag.error=TAG_ERROR_OVER_TIME;
			}
			cip_update_write_history(_pbb_write_taglist);
			SignleList_Clear(&_pbb_write_taglist,RecordTagStruct);
			ClearTagList(&_pbb_signle_req_tag_list);
		}
		if(list!=NULL)
		{
			for(tag=list;tag;tag=list)
			{
				list = tag->next;
				SignleList_Push(&_pbb_write_taglist,tag,RecordTagStruct);
				num=get_real_tag_name(pbuf,tag->tagname);
				read_tag=GetTagFromListByTagname(&_pbb_client->MutileTagList,pbuf);
				if(read_tag==NULL)//标签列表中没有需要请求的标签，
				{
					tag->tag.error = TAG_ERROR_NOT_INLIST;
					continue;
				}
				else if(IsRightCIP_Type(tag->tag.type)==0)
				{
					tag->tag.error = TAG_ERROR_TYPE_ERROR;
					continue;
				}else if(read_tag->error !=0)
				{
					tag->tag.error = read_tag->error;
					continue;
				}

				mtag=GetTagFromListByTagname(&_pbb_signle_req_tag_list,pbuf);
				if(num<0 && mtag ==NULL)
				{
					if(tag->tag.type!=read_tag->type)
					{
						tag->tag.error = TAG_ERROR_TYPE_ERROR;
						continue;
					}
					new_tag=CreatTagStruct(pbuf,tag->tag.rw_flag);//创建一个写标签
					new_tag->error=0;
					new_tag->type=tag->tag.type;
					CipValueCopy(&new_tag->value ,&tag->tag.value,new_tag->type);
					tag->mtag = new_tag;
					TagStructPush(&_pbb_signle_req_tag_list,new_tag);
				}
				else if(num<0 && mtag !=NULL)
				{
					if(tag->tag.type!=mtag->type)
					{
						tag->tag.error = TAG_ERROR_TYPE_ERROR;
						continue;
					}
					CipValueCopy(&mtag->value ,&tag->tag.value,mtag->type);
					tag->mtag=mtag;
				}
				else if(num>=0 && mtag == NULL)
				{
					if(tag->tag.type != CIP_TYPE_BOOL)
					{
						tag->tag.error = TAG_ERROR_TYPE_ERROR;
						continue;
					}
					new_tag=CloneTagStruct(read_tag);//复制一个read_tag
					//CipValueCopy(&new_tag->value,&read_tag->value,read_tag->type);
					if(set_cip_bit_value(&new_tag->value,read_tag->type,num,tag->tag.value.bool)==0)
					{
						new_tag->rw_flag=tag->tag.rw_flag;
						tag->mtag = new_tag;
						TagStructPush(&_pbb_signle_req_tag_list,new_tag);
					}
					else
					{
						tag->tag.error=TAG_ERROR_BIT_NUM_ERROR;
						FreeTagStruct(new_tag);
					}
				}else if(num>=0 && mtag != NULL)
				{
					if(tag->tag.type != CIP_TYPE_BOOL)
					{
						tag->tag.error = TAG_ERROR_TYPE_ERROR;
						continue;
					}
					if(set_cip_bit_value(&mtag->value,read_tag->type,num,tag->tag.value.bool)==0)
					{
						tag->mtag = mtag;
					}
					else
					{
						tag->tag.error=TAG_ERROR_BIT_NUM_ERROR;
					}
				}
			}

			if(_pbb_signle_req_tag_list)
			{
				AddSignleRequstTaglist(_pbb_client,_pbb_signle_req_tag_list);
			}
			else
			{
				if(_pbb_write_taglist)
				{
					cip_update_write_history(_pbb_write_taglist);
					SignleList_Clear(&_pbb_write_taglist,RecordTagStruct);
				}
				pthread_mutex_unlock(&_pbb_write_tag_mutex);
			}
		}else
		{
			pthread_mutex_unlock(&_pbb_write_tag_mutex);
		}
	}

}

