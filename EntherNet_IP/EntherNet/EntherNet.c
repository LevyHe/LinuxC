// 
// 创建人： levy
// 创建时间：May 25, 2017
// 功能：EntherNet.c,由与Linux 系统与CIP协议都是使用的小端模式，在本程序中很多数据类型转字节数组利用了取地址直接转换的方法
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         May 25, 2017                  levy          初版
// 

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "EntherNet.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
const char * CIP_SENDER_CONTEXT = "HY_DTU02";
const int MAX_CIP_MESSAGE_LEN = 504;
static CIP * _cip_client_list=NULL;
static pthread_mutex_t _cip_client_lsit_mutex = PTHREAD_MUTEX_INITIALIZER;

const CIP_BYTE _cip_typelist[]={CIP_TYPE_BOOL,CIP_TYPE_SINT,CIP_TYPE_INT,CIP_TYPE_DINT,CIP_TYPE_LINT,CIP_TYPE_USINT,CIP_TYPE_UINT,CIP_TYPE_UDINT,CIP_TYPE_ULINT,CIP_TYPE_REAL};

static void *(*cip_malloc)(size_t sz) = malloc;
static void (*cip_free)(void *ptr) = free;
static void *cip_ClientThread(void *arp);
static int SendRegisterSession(CIP *cip);
//检查 字符串 in 中是否包含有效的标签名
static int StringHasVaidTag(const char *in)
{
	if(in == NULL)return 0;
    while (*in && (*in <= 0x20))
    {
        in++;
    }
    return *in=='\0'?0:1;
}
static const char * skip_blank(const char * in)
{
	while(in && *in && (*in <=0x20)){
		in ++;
	}
	return in;
}

static char *GetValidTag(char *to,const char*from,size_t size)
{
	size_t i;
	from = skip_blank(from);
	for(i=0;i<size && from[i]>0x20;i++)
	{
		to[i]=from[i];
	}
	to[i]=0;
	return to;
}

static int tag_strncasecmp(const char *s1,const char *s2,size_t size)
{
	   if (s1==NULL)
	    {
	        return s2==NULL ? 0 : *s2;
	    }
	    for(; toupper(*s1) == toupper(*s2); s1++, s2++)
	    {
	        if (*s1 == '\0')
	        {
	            return 0;
	        }
	    }

	    return toupper(*s1) - toupper(*s2);
}

static inline void tv_MillisecondsSet(struct timeval *tv, int ms)
{
	tv->tv_sec =ms/1000;
	tv->tv_usec =(ms%1000)*1000;
}

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

//初始化一个数据包的包头
static void makeheaderstruct(HeaderStruct *head)
{
	head->Cmd = 0x0065;
	head->Status =0;
	head->Length = 0x04;
	head->SessionHandle = 0x00;
	memcpy(head->SenderContext,CIP_SENDER_CONTEXT,sizeof(head->SenderContext));
	head->Options = 0x0000;
}

static void __weak_signle_fptr(void *arg )
{
	CIP *cip = (CIP*)arg;

	ClearTagList(&cip->SignleRepTag);
}

/*XXX: TagStruct处理的相关函数*/
//copy cip value
void CipValueCopy(CIP_VALUE *to ,const CIP_VALUE *from,CIP_BYTE type)
{
	memset(to,0,sizeof(CIP_VALUE));
	switch(type)
	{
		case 0xC1:
			to->bool = from->bool;
		   break;
		case 0xC2:
		case 0xC6:
			to->usint = from->usint;
		   break;
		case 0xC3:
		case 0xC7:
			to->uint = from->uint;
		   break;
		case 0xC4:
		case 0xC8:
			to->dint = from->dint;
		   break;
		case 0xC5:
		case 0xC9:
			to->lint = from->lint;
		   break;
		case 0xCA:
			to->real = from->real;
		   break;
		default:
			memcpy(to,from,sizeof(CIP_VALUE));
		   break;
	}
}

//比较 val1与val2 val1>va2 return 1, val1<va2 return -1,val1=va2 return 0
int CIPValueCmp(CIP_VALUE *val1 ,const CIP_VALUE *val2,CIP_BYTE type)
{
	int res=0;
	switch(type)
	{
		case 0xC1:
			res = Value_CMP(val1->bool,val2->bool);
		   break;
		case 0xC2:
			res = Value_CMP(val1->sint,val2->sint);
			break;
		case 0xC3:
			res = Value_CMP(val1->iint,val2->iint);
			break;
		case 0xC4:
			res = Value_CMP(val1->dint,val2->dint);
			break;
		case 0xC5:
			res = Value_CMP(val1->lint,val2->lint);
			break;
		case 0xC6:
			res = Value_CMP(val1->usint,val2->usint);
		   break;
		case 0xC7:
			res = Value_CMP(val1->uint,val2->uint);
		   break;
		case 0xC8:
			res = Value_CMP(val1->udint,val2->udint);
		   break;
		case 0xC9:
			res = Value_CMP(val1->ulint,val2->ulint);
		   break;
		case 0xCA:
			res = Value_CMP(val1->real,val2->real);
		   break;
		default:
			memcpy(val1->buf,val2->buf,sizeof(CIP_VALUE));
		   break;
	}
	return res;
}

//将给定的CIP_VALUE打印到obuf，请确保obuf有足够的空间来保存输出去的字符串
char *cip_value2string(char * obuf,CIP_VALUE *val,CIP_BYTE type)
{
	switch(type)
	{
		case 0xC1:
			sprintf(obuf,"%d",val->bool);
		   break;
		case 0xC2:
			sprintf(obuf,"%hhd",val->sint);
		   break;
		case 0xC3:
			sprintf(obuf,"%hd",val->iint);
			break;
		case 0xC4:
			sprintf(obuf,"%d",val->dint);
			break;
		case 0xC5:
			sprintf(obuf,"%lld",(long long)val->lint);
			break;
		case 0xC6:
			sprintf(obuf,"%hhu",val->usint);
			break;
		case 0xC7:
			sprintf(obuf,"%hu",val->uint);
		   break;
		case 0xC8:
			sprintf(obuf,"%u",val->udint);
		   break;

		case 0xC9:
			sprintf(obuf,"%llu",(unsigned long long)val->ulint);
		   break;
		case 0xCA:
			sprintf(obuf,"%f",val->real);
		   break;
		default:
			memcpy(obuf,val,sizeof(CIP_VALUE));
		   break;
	}
	return obuf;
}

CIP_VALUE * cip_get_value_from_string(CIP_VALUE *val ,const char * str ,CIP_BYTE type)
{
	switch(type)
		{
			case 0xC1:
				val->bool = atoi(str)?1:0;
			   break;
			case 0xC2:
				val->sint=(CIP_SINT)((str)==NULL?0:atoi(str));
			   break;
			case 0xC3:
				val->iint=(CIP_INT)((str)==NULL?0:atoi(str));
				break;
			case 0xC4:
				val->dint=(CIP_DINT)((str)==NULL?0:atoi(str));
				break;
			case 0xC5:
				val->lint=(CIP_LINT)((str)==NULL?0:atoll(str));
				break;
			case 0xC6:
				val->usint=(CIP_USINT)((str)==NULL?0:strtoul(str,NULL,0));
				break;
			case 0xC7:
				val->uint=(CIP_UINT)((str)==NULL?0:strtoul(str,NULL,0));
			   break;
			case 0xC8:
				val->udint=(CIP_UDINT)((str)==NULL?0:strtoul(str,NULL,0));
			   break;
			case 0xC9:
				val->ulint=(CIP_ULINT)((str)==NULL?0:strtoull(str,NULL,0));
			   break;
			case 0xCA:
				val->iint=(CIP_INT)((str)==NULL?0:atof(str));
			   break;
			default:
				memcpy(val,str,sizeof(CIP_VALUE));
			   break;
		}
		return val;
}

//获取 CIP_VALUE中指定类型type，指定位地址bitnum，@return -1 错误, 0 or 1 表示成功
int get_cip_bit_value(CIP_VALUE* val,CIP_BYTE type, int bitnum)
{
	int res=0;
	switch(type)
	{
        case 0xC2:
        case 0xC6:
            if (bitnum < 8 && bitnum >= 0)
            {
            	res=(val->usint & (1 << bitnum)) == 0 ? 0 : 1;
            }
            else
            {
            	res=-1;
            }
            break;
        case 0xC3:
        case 0xC7:
            if (bitnum < 16 && bitnum >= 0)
            {
            	res=(val->uint & (1 << bitnum)) == 0 ? 0 : 1;
            }
            else
            {
            	res=-1;
            }
            break;
        case 0xC4:
        case 0xC8:
            if (bitnum < 32 && bitnum >= 0)
            {
            	res=(val->udint & (1 << bitnum)) == 0 ? 0 : 1;
            }
            else
            {
            	res=-1;
            }
            break;
        case 0xC5:
        case 0xC9:
            if (bitnum < 64 && bitnum >= 0)
            {
            	res=(val->ulint & (1 << bitnum)) == 0 ? 0 : 1;
            }
            else
            {
            	res=-1;
            }
            break;
        default:
            res=-1;
            break;
	}
	return res;
}

int set_cip_bit_value(CIP_VALUE* val,CIP_BYTE type, int bitnum,CIP_BOOL bool_val)
{
	int res=0;
	switch(type)
	{
        case 0xC2:
        case 0xC6:
            if (bitnum < 8 && bitnum >= 0)
            {
            	CIP_USINT b8;
            	b8 = val->usint;
            	val->usint = bool_val ? b8 | (1 << bitnum) : b8 & (~(1 << bitnum));
            	res=0;
            }
            else
            {
            	res=-1;
            }
            break;
        case 0xC3:
        case 0xC7:
            if (bitnum < 16 && bitnum >= 0)
            {
            	CIP_UINT b16;
            	b16 = val->uint;
            	val->usint = bool_val ? b16 | (1 << bitnum) : b16 & (~(1 << bitnum));
            	res=0;
            }
            else
            {
            	res=-1;
            }
            break;
        case 0xC4:
        case 0xC8:
            if (bitnum < 32 && bitnum >= 0)
            {
            	CIP_UDINT b32;
            	b32 = val->uint;
            	val->usint = bool_val ? b32 | (1 << bitnum) : b32 & (~(1 << bitnum));
            	res=0;
            }
            else
            {
            	res=-1;
            }
            break;
        case 0xC5:
        case 0xC9:
            if (bitnum < 64 && bitnum >= 0)
            {
            	CIP_ULINT b64;
            	b64 = val->uint;
                val->usint = bool_val ? b64 | (1 << bitnum) : b64 & (~(1 << bitnum));
                res=0;
            }
            else
            {
            	res=-1;
            }
            break;
        default:
            res=-1;
            break;
	}
	return res;
}

int IsRightCIP_Type(CIP_BYTE byte)
{
	for(int i=0;i<sizeof(_cip_typelist);i++)
	{
		if(_cip_typelist[i]==byte)
			return 1;
	}
	return 0;
}

//c创建一个新的标签
TagStruct* CreatTagStruct(const char* tagname , int rw_flag)
{
	if(!StringHasVaidTag(tagname))
	{
		return NULL;
	}
	TagStruct *tag = cip_malloc(sizeof(TagStruct));
	GetValidTag(tag->tag,tagname,sizeof(tag->tag));
	tag->type =0;
	tag->rw_flag = rw_flag;
	tag->next = NULL;
	tag->prev = NULL;
	tag->error = 0;
	return tag;
}

//克隆一个标签
TagStruct* CloneTagStruct(const TagStruct *ftag)
{
	TagStruct *tag = cip_malloc(sizeof(TagStruct));
	memcpy(tag,ftag,sizeof(TagStruct));
	return tag;
}

//向ptag指向的链表中压入一个标签
void TagStructPush(TagStruct ** ptag ,TagStruct *tag)
{
	TagStruct * ntag;
	ntag = *ptag;
	tag->next = NULL;
	if(ntag == NULL)
	{
		tag->prev = NULL;
		*ptag = tag;
	}else
	{
		while(ntag->next)
		{
			ntag = ntag->next;
		}
		tag->prev = ntag;
		ntag->next = tag;
	}
}

//从ptag指向的链表中推出一个标签
TagStruct * TagStructPop(TagStruct ** ptag)
{
	TagStruct * ntag = *ptag;
	if(ntag!=NULL)
	{
		*ptag = ntag->next;
		if(*ptag)
		{
			(*ptag)->prev = NULL;
		}
	}
	return ntag;
}

void FreeTagStruct(TagStruct *tag)
{
	cip_free(tag);
}

//从taglist获取与dtag具有相同标签名和读写标志的标签
TagStruct *GetTagFromListByTag(TagStruct ** taglist ,TagStruct * dtag)
{
	TagStruct * tag ;
	for(tag = *taglist;tag;tag=tag->next)
	{
		if(tag->rw_flag == dtag->rw_flag && tag_strncasecmp(tag->tag,dtag->tag,128)==0)
		{
			break;
		}
	}
	return tag;
}

TagStruct *GetTagFromListByTagname(TagStruct ** taglist ,const char * tagname)
{
	TagStruct * tag ;
	for(tag = *taglist;tag;tag=tag->next)
	{
		if(tag_strncasecmp(tag->tag,tagname,128)==0)
		{
			break;
		}
	}
	return tag;
}


//从taglist删除第一个与dtag具有相同标签名与rw_flag的标签
void RemoveTagFromTaglistByTag(TagStruct ** taglist ,TagStruct * dtag)
{

	TagStruct * ltag ;
	for(ltag = * taglist;ltag;ltag=ltag->next)
	{
		if(ltag->rw_flag == dtag->rw_flag && tag_strncasecmp(ltag->tag,dtag->tag,128)==0)//存在标签名相同的标签
		{

			if(ltag->prev)	//not the first element
			{
				ltag->prev->next = ltag->next;
			}
			if(ltag->next)	//not  the last element
			{
				ltag->next->prev = ltag->prev;
			}
			if(ltag == * taglist)
			{
				* taglist=ltag->next;
			}
			ltag->prev = ltag->next = NULL;
			cip_free(ltag);
			break;
		}
	}
}

//清空taglist所有的标签，并释放资源
void ClearTagList(TagStruct ** taglist)
{
	TagStruct * ntag ;
	TagStruct * rtag ;
	for(rtag = * taglist;rtag;rtag=ntag)
	{
		ntag = rtag->next;
		cip_free(rtag);
	}
	* taglist=NULL;
}

/*XXX:EntherNet/IP 结构体与字节数组之间的转换相关函数*/
//ForwardOpenRequstStructToBytes
int  ForwardOpenRequstStructToBytes(ForwardOpenRequstStruct *fr, CIP_BYTE * obuf)
{
	memcpy(obuf,fr,sizeof(ForwardOpenRequstStruct));
	return sizeof(ForwardOpenRequstStruct);
}

//将标签名字符串，转华为Symbol排列的字节数组
int TagstringToSymbolansiBytes(CIP_BYTE * obytes, char * tag,int max_size)
{
	int count =0 ;
	int len = 0;
	char * f_dot;
	for(char * next=tag; next; next = f_dot[0]!=0 ? f_dot+1 : NULL)
	{
		f_dot=strchrnul(next,'.');
		len = f_dot-next;			//计算有标签名长度
		if(count+len>max_size)break;//标签名长度过长
		char * left_b = memchr(next,'[',len);
		if(left_b)
		{
			//char* right_b = memchr(left_b,']',len);
			int n_len=left_b-next;
			CIP_BYTE memid = atoi(left_b+1);
			obytes[count++] = 0x91;//SymbolAnsiStruct.SegmentType
			obytes[count++]=n_len;//SymbolAnsiStruct.SegmentSize
			memcpy(&obytes[count],next,n_len);//SymbolAnsiStruct.SegmentTag
			count+=n_len;
			if(n_len%2==1)//标签名长度为奇数，在末尾补零
			{
				obytes[count++]=0;
			}
			obytes[count++] = 0x28;//SymbolAnsiStruct.IDType
			obytes[count++] = memid;//SymbolAnsiStruct.MemeberID
		}
		else
		{
			obytes[count++] = 0x91;//SymbolAnsiStruct.SegmentType
			obytes[count++]=len;//SymbolAnsiStruct.SegmentSize
			memcpy(&obytes[count],next,len);//SymbolAnsiStruct.SegmentTag
			count+=len;
			if(len%2==1)//标签名长度为奇数，在末尾补零
			{
				obytes[count++]=0;
			}
			//obytes[count++] = 0;//SymbolAnsiStruct.IDType
			//obytes[count++] = 0;//SymbolAnsiStruct.MemeberID

		}
	}

	return count;
}

//嵌入式服务请求结构体转Bytes
int  EmbeddedRequstToBytes(const EmbeddedRequstStruct *st,CIP_BYTE* obuf)
{
	int count=0;
	obuf[count++]=st->Cmd;
	obuf[count++]=st->EpathLength;
	memcpy(obuf+count,st->SymbolTag,st->EpathLength*2);
	count += st->EpathLength*2;
//	SymbolAnsiStruct *sa=st->SymbolList;
//	while(sa)
//	{
//		obuf[count++]=sa->SegmentType;
//		obuf[count++]=sa->SegmentSize;
//		if(sa->SegmentSize%2==0)//标签名长度为偶数
//		{
//			memcpy(obuf+count,sa->SegmentTag,sa->SegmentSize);
//			count+=sa->SegmentSize;
//		}else//标签名长度为奇数
//		{
//
//			memcpy(obuf+count,sa->SegmentTag,sa->SegmentSize+1);
//			count+=sa->SegmentSize+1;
//		}
//		obuf[count++]=sa->IDType;
//		obuf[count++]=sa->MemeberID;
//		sa	= sa->next;
//	}
	if(st->Cmd == 0x4C)
	{
		memcpy(obuf+count,&st->SpecificData,sizeof(CIP_UINT));
		count+=sizeof(CIP_UINT);
	}
	else if(st->Cmd==0x4D)
	{
		obuf[count++]=st->DataType;
		obuf[count++]=st->Zero2;
		memcpy(obuf+count,&st->SpecificData,sizeof(CIP_UINT));
		count+=sizeof(CIP_UINT);
		switch(st->DataType)
		{
			case 0xC1:
				memcpy(obuf+count,st->value,sizeof(CIP_BOOL));
				count+=sizeof(CIP_BOOL);
			   break;
			case 0xC2:
			case 0xC6:
				obuf[count++]=st->value[0];
			   break;
			case 0xC3:
			case 0xC7:
				memcpy(obuf+count,st->value,sizeof(CIP_INT));
				count+=sizeof(CIP_INT);
			   break;
			case 0xC4:
			case 0xC8:
				memcpy(obuf+count,st->value,sizeof(CIP_DINT));
				count+=sizeof(CIP_DINT);
			   break;
			case 0xC5:
			case 0xC9:
				memcpy(obuf+count,st->value,sizeof(CIP_LINT));
				count+=sizeof(CIP_LINT);
			   break;
			case 0xCA:
				memcpy(obuf+count,st->value,sizeof(CIP_REAL));
				count+=sizeof(CIP_REAL);
			   break;
			default:
			   break;
		}
	}else
	{
		return -1;
	}
	return count;
}

int GetEmbeddedRequstStructSizes(const EmbeddedRequstStruct *st)
{
	int count=0;

	count += 2+st->EpathLength*2;

	if(st->Cmd == 0x4C)
	{
		count+=sizeof(CIP_UINT);
	}
	else if(st->Cmd==0x4D)
	{
		count+=sizeof(CIP_UINT)+2;
		switch(st->DataType)
		{
			case 0xC1:
				count+=sizeof(CIP_BOOL);
			   break;
			case 0xC2:
			case 0xC6:
				count++;
			   break;
			case 0xC3:
			case 0xC7:
				count+=sizeof(CIP_INT);
			   break;
			case 0xC4:
			case 0xC8:
				count+=sizeof(CIP_DINT);
			   break;
			case 0xC5:
			case 0xC9:
				count+=sizeof(CIP_LINT);
			   break;
			case 0xCA:
				count+=sizeof(CIP_REAL);
			   break;
			default:
			   break;
		}
	}else
	{
		return -1;
	}
	return count;
}

//将bytes转为嵌入式服务服务响应结构体
int BytesToEmbeddedResponse(EmbeddedResponseStruct *st ,const CIP_BYTE * bytes)
{
	st->Cmd = bytes[0];
	st->Zero1 = bytes[1];
	st->StatusCode = bytes[2];
	if(st->Cmd == 0xCC)	//读数据标签响应
	{
		if(st->StatusCode == 0)//正常响应
		{
			st->AddStatusNum = bytes[3];
			st->DataType = bytes[4];
			st->Zero2= bytes[5];
			switch(st->DataType)
			{
				case 0xC1:
					memcpy(&st->value,&bytes[6],sizeof(CIP_BOOL));
				   break;
				case 0xC2:
				case 0xC6:
					st->value.sint=bytes[6];
				   break;
				case 0xC3:
				case 0xC7:
					memcpy(&st->value,&bytes[6],sizeof(CIP_INT));
				   break;
				case 0xC4:
				case 0xC8:
					memcpy(&st->value,&bytes[6],sizeof(CIP_DINT));
				   break;
				case 0xC5:
				case 0xC9:
					memcpy(&st->value,&bytes[6],sizeof(CIP_LINT));
				   break;
				case 0xCA:
					memcpy(&st->value,&bytes[6],sizeof(CIP_REAL));
				   break;
				default:
				   break;
			}
		}
		else
		{
			st->AddStatusNum = bytes[3];
			if(st->AddStatusNum>0)
			{
				memcpy(st->AddStatus,&bytes[4],st->AddStatusNum*2);
			}
		}
	}
	else if(st->Cmd == 0xCD)
	{

		if(st->StatusCode == 0 && bytes[3] == 0)
		{
			st->AddStatusNum =  bytes[3];
		}
		else
		{
			st->AddStatusNum =bytes[3];
			if(st->AddStatusNum>0)
			{
				memcpy(st->AddStatus,&bytes[4],st->AddStatusNum*2);
			}
		}
	}
	else
	{
		return -1;
	}
	return 0;
}

//将多包发送服务结构体转为字节
int MultipleServiceRequstToBytes(const MultipleServiceRequstStruct *st,CIP_BYTE* obuf)
{
	int count = 0;
	memcpy(obuf+count,&st->ServiceNum,sizeof(CIP_UINT));
	count += sizeof(CIP_UINT);
	for(int i = 0; i < st->ServiceNum; i++)
	{
		memcpy(obuf+count,&st->ServiceOffset[i],sizeof(CIP_UINT));
		count += sizeof(CIP_UINT);
	}
   for (int i = 0; i < st->ServiceNum; i++)
   {
	   count+=EmbeddedRequstToBytes(&st->ServiceList[i],obuf+count);
   }
	   return count;
}

//将字节转为多包服务响应结构体
int BytesToMultipleServiceReponse(MultipleServiceReponseStruct *st,const CIP_BYTE * bytes)
{
	st->ReponseNum = BytesToValue_CIP_UINT(bytes,0);
	if(st->ReponseNum > 0)
	{
		memcpy(st->ReponseOffset,bytes+2,st->ReponseNum*2);
		for(int i = 0; i < st->ReponseNum; i++)
		{
			//int len = i+1 < st->ReponseNum ? st->ReponseOffset[i+1] - st->ReponseOffset[i] : 0 ;
			BytesToEmbeddedResponse(&st->ReponseList[i],bytes+st->ReponseOffset[i]);

		}

	}
	return 0;
}

//将消息路由请求转为bytes
int MessageRouterRequstToBytes(const MessageRouterRequstStrust *st, CIP_BYTE * obuf)
{
	int count = 0;
	obuf[count++] = st->ServiceCode;
	obuf[count++] = st->EpathSize;
	memcpy(obuf + count,st->Epath,st->EpathSize*2);
	count += st->EpathSize*2;
	memcpy(obuf+count,st->RequstData,st->rdata_len);
	count += st->rdata_len;
	return count;
}

//将bytes转为消息路由结构体
int BytesToMessageRouterReponse(MessageRouterReponseStrust * st ,const CIP_BYTE * bytes , int len)
{
	st->RServiceCode = bytes[0];
	st->Resered = bytes[1];
	st->Status = bytes[2];
	st->AddStatusSize = bytes[3];
	if(st->AddStatusSize > 0)
	{
		memcpy(st->AddStatus,&bytes[4],st->AddStatusSize*2);
	}
	st->rdata_len = len - 4 - st->AddStatusSize*2;
	memcpy(st->ReponseData,&bytes[4+st->AddStatusSize*2],st->rdata_len);
	return 0;
}

//将连接消息结构体转为bytes
int ConnectedMessageStructToBytes(ConnectedDataPacketStruct *st, CIP_BYTE*obuf)
{
	int count=0;
	count+=sizeof(HeaderStruct);
	memcpy(obuf+count,&st->InterfaceHandle,sizeof(CIP_UDINT));
	count += sizeof(CIP_UDINT);
	memcpy(obuf+count,&st->TimeOut,sizeof(CIP_UINT));
	count += sizeof(CIP_UINT);
	memcpy(obuf+count,&st->ItemCount,sizeof(CIP_UINT));
	count += sizeof(CIP_UINT);
	memcpy(obuf+count,&st->AddressTypeID,sizeof(CIP_UINT));
	count += sizeof(CIP_UINT);
	memcpy(obuf+count,&st->AddressLength,sizeof(CIP_UINT));
	count += sizeof(CIP_UINT);
	memcpy(obuf+count,&st->ConnectionID,sizeof(CIP_UDINT));
	count += sizeof(CIP_UDINT);
	memcpy(obuf+count,&st->DataTypeID,sizeof(CIP_UINT));
	count += sizeof(CIP_UINT);
	memcpy(obuf+count,&st->DataLength,sizeof(CIP_UINT));
	count += sizeof(CIP_UINT);
	memcpy(obuf+count,&st->ConnetionNum,sizeof(CIP_UINT));
	count += sizeof(CIP_UINT);
	memcpy(obuf+count,st->MessageRouterData,st->DataLength -2);
	count += st->DataLength -2;
	st->Header.Length = count - sizeof(HeaderStruct);
	memcpy(obuf,&st->Header,sizeof(HeaderStruct));
	return count;
}

//将bytes转为连接的消息结构体
int BytesToConnnetedMessageStruct(ConnectedDataPacketStruct* st ,const CIP_BYTE * bytes,int len)
{
   if (len < 46) return -1;
   int count=0;
   memcpy(&st->Header,bytes,sizeof(HeaderStruct));
   count += sizeof(HeaderStruct);
   st->InterfaceHandle = BytesToValue_CIP_UDINT(bytes,count);
   count+=sizeof(CIP_UDINT);
   st->TimeOut = BytesToValue_CIP_UINT(bytes,count);
   count+=sizeof(CIP_UINT);
   st->ItemCount = BytesToValue_CIP_UINT(bytes,count);
   count+=sizeof(CIP_UINT);
   st->AddressTypeID = BytesToValue_CIP_UINT(bytes,count);
   count+=sizeof(CIP_UINT);
   st->AddressLength = BytesToValue_CIP_UINT(bytes,count);
   count+=sizeof(CIP_UINT);
   st->ConnectionID = BytesToValue_CIP_UDINT(bytes,count);
   count+=sizeof(CIP_UDINT);
   st->DataTypeID = BytesToValue_CIP_UINT(bytes,count);
   count+=sizeof(CIP_UINT);
   st->DataLength = BytesToValue_CIP_UINT(bytes,count);
   count+=sizeof(CIP_UINT);
   st->ConnetionNum = BytesToValue_CIP_UINT(bytes,count);
   count+=sizeof(CIP_UINT);
   if(st->DataTypeID != 0x00B1)
	   return -1;
   if(st->DataLength > (len - count + 2))
	   return -1;
   memcpy(st->MessageRouterData,&bytes[count],st->DataLength-2);
   return 0;
}

//将UCMM结构体转为bytes
int UCMMStructToBytes(UnConnectedDataPacketStruct*st,CIP_BYTE * obuf)
{
	int count=0;
	count+=sizeof(HeaderStruct);
	memcpy(obuf+count,&st->InterfaceHandle,sizeof(CIP_UDINT));
	count += sizeof(CIP_UDINT);
	memcpy(obuf+count,&st->TimeOut,sizeof(CIP_UINT));
	count += sizeof(CIP_UINT);
	memcpy(obuf+count,&st->ItemCount,sizeof(CIP_UINT));
	count += sizeof(CIP_UINT);
	memcpy(obuf+count,&st->AddressTypeID,sizeof(CIP_UINT));
	count += sizeof(CIP_UINT);
	memcpy(obuf+count,&st->AddressLength,sizeof(CIP_UINT));
	count += sizeof(CIP_UINT);
	memcpy(obuf+count,&st->DataTypeID,sizeof(CIP_UINT));
	count += sizeof(CIP_UINT);
	memcpy(obuf+count,&st->DataLength,sizeof(CIP_UINT));
	count += sizeof(CIP_UINT);
	memcpy(obuf+count,st->MessageRouterData,st->DataLength);
	count += st->DataLength;
	st->Header.Length = count - sizeof(HeaderStruct);
	memcpy(obuf,&st->Header,sizeof(HeaderStruct));
	return count;
}

//将bytes转UCMM结构体
int BytesToUCMMStruct(UnConnectedDataPacketStruct * st , const CIP_BYTE *bytes,int len)
{

	 if (len < 40) return -1;
	   int count=0;
	   memcpy(&st->Header,bytes,sizeof(HeaderStruct));
	   count += sizeof(HeaderStruct);
	   st->InterfaceHandle = BytesToValue_CIP_UDINT(bytes,count);
	   count+=sizeof(CIP_UDINT);
	   st->TimeOut = BytesToValue_CIP_UINT(bytes,count);
	   count+=sizeof(CIP_UINT);
	   st->ItemCount = BytesToValue_CIP_UINT(bytes,count);
	   count+=sizeof(CIP_UINT);
	   st->AddressTypeID = BytesToValue_CIP_UINT(bytes,count);
	   count+=sizeof(CIP_UINT);
	   st->AddressLength = BytesToValue_CIP_UINT(bytes,count);
	   count+=sizeof(CIP_UINT);
	   if(st->AddressTypeID !=0 || st->AddressLength != 0) return -1;
	   st->DataTypeID = BytesToValue_CIP_UINT(bytes,count);
	   count+=sizeof(CIP_UINT);
	   if(st->DataTypeID != 0x00B2) return -1;
	   st->DataLength = BytesToValue_CIP_UINT(bytes,count);
	   count+=sizeof(CIP_UINT);
	   if(st->DataLength > len - count) return -1;
	   memcpy(st->MessageRouterData,&bytes[count],st->DataLength);
	   return 0;
}

/*XXX:cip 客户端内部标签处理函数*/
//添加单个单次请求读标签服务，每次只能处理一个
int AddSignleReadTagRequst(CIP*cip,const const char * tagname)
{
	pthread_mutex_lock(&cip->SignleTag_Mutex);
	TagStruct *tag = cip_malloc(sizeof(TagStruct));
	tag->error = 0;
	tag->rw_flag = 0;
	GetValidTag(tag->tag,tagname,sizeof(tag->tag));
	TagStructPush(&cip->SignleReqTag,tag);
	pthread_cond_signal(&cip->SignleTag_Cond);
	pthread_mutex_unlock(&cip->SignleTag_Mutex);
	return 0;
}

//添加单个单次请求写标签服务，每次只能处理一个
int AddSignleWriteTagRequst(CIP*cip,const const char * tagname,CIP_BYTE type,CIP_VALUE *value)
{
	pthread_mutex_lock(&cip->SignleTag_Mutex);
	TagStruct *tag = cip_malloc(sizeof(TagStruct));
	tag->error = 0;
	tag->rw_flag = 1;
	tag->type = type;
	tag->next = NULL;
	CipValueCopy(&tag->value,value,type);
	GetValidTag(tag->tag,tagname,sizeof(tag->tag));
	TagStructPush(&cip->SignleReqTag,tag);
	pthread_cond_signal(&cip->SignleTag_Cond);
	pthread_mutex_unlock(&cip->SignleTag_Mutex);
	return 0;
}

int AddSignleRequstTaglist(CIP * cip , const TagStruct * list)
{
	pthread_mutex_lock(&cip->SignleTag_Mutex);
	for(const TagStruct * tag = list;tag;tag=tag->next)
	{
		TagStruct* ctag = CloneTagStruct(tag);
		TagStructPush(&cip->SignleReqTag,ctag);
	}
	if(cip->SignleReqTag)
	{
		pthread_cond_signal(&cip->SignleTag_Cond);
	}
	pthread_mutex_unlock(&cip->SignleTag_Mutex);
	return 0;
}

int ClearSignleTagList(CIP*cip)
{
	TagStruct * tag ;
	TagStruct * rtag ;
	pthread_mutex_lock(&cip->SignleTag_Mutex);
	for(rtag = cip->SignleReqTag;rtag;rtag=tag)
	{
		tag = rtag->next;
		cip_free(rtag);
	}
	cip->SignleReqTag=NULL;
	for(rtag = cip->SignleRepTag;rtag;rtag=tag)
	{
		tag = rtag->next;
		cip_free(rtag);
	}
	cip->SignleRepTag=NULL;
	pthread_mutex_unlock(&cip->SignleTag_Mutex);
	return 0;
}

//添加多次请求的读标签服务
int AddMutileReadTagRequst(CIP * cip ,const char * tagname)
{
	TagStruct * tag ;
	TagStruct * ltag;
	if(!StringHasVaidTag(tagname))
	{
		return -1;
	}
	pthread_mutex_lock(&cip->MutileTagList_Mutex);
	tag = cip_malloc(sizeof(TagStruct));
	tag->next = NULL;
	tag->prev = NULL;
	tag->error = 0;
	tag->rw_flag = 0;
	GetValidTag(tag->tag,tagname,sizeof(tag->tag));
	if(cip->MutileTagList == NULL)	//标签列表为空
	{
		cip->MutileTagList = tag;
		pthread_mutex_unlock(&cip->MutileTagList_Mutex);
		return 0;
	}
	for(ltag = cip->MutileTagList;ltag->next != NULL;)
	{
		ltag=ltag->next;
	}
	ltag->next = tag;
	tag->prev = ltag;
	pthread_mutex_unlock(&cip->MutileTagList_Mutex);
	return 0;
}

//添加多次请求的写标签服务
int AddMutileWriteTagRequst(CIP * cip ,const char * tagname,CIP_BYTE type,CIP_VALUE *value)
{
	TagStruct * tag ;
	TagStruct * ltag;
	if(!StringHasVaidTag(tagname) || value==NULL )
	{
		return 0;
	}
	pthread_mutex_lock(&cip->MutileTagList_Mutex);
	tag = cip_malloc(sizeof(TagStruct));
	tag->next = NULL;
	tag->prev = NULL;
	tag->error = 0;
	tag->type = type;
	tag->rw_flag = 1;
	CipValueCopy(&tag->value,value,type);
	GetValidTag(tag->tag,tagname,sizeof(tag->tag));
	if(cip->MutileTagList == NULL)	//标签列表为空
	{
		cip->MutileTagList = tag;
		pthread_mutex_unlock(&cip->MutileTagList_Mutex);
		return 0;
	}
	for(ltag = cip->MutileTagList;ltag->next != NULL;)
	{
		ltag=ltag->next;
	}
	ltag->next = tag;
	tag->prev = ltag;
	pthread_mutex_unlock(&cip->MutileTagList_Mutex);
	return 0;
}

//更新多次请求的写标签的值
int UpdateMutileTagValue(CIP * cip ,const char * tagname,CIP_BYTE type,CIP_VALUE *value)
{
	TagStruct * ltag ;
	if(cip->MutileTagList == NULL)
	{
		return -1;
	}
	for(ltag = cip->MutileTagList;ltag;ltag=ltag->next)
	{
		if(ltag->rw_flag == 1 && tag_strncasecmp(ltag->tag,tagname,128)==0)//存在标签名相同的标签
		{
			ltag->type = type;
			CipValueCopy(&ltag->value,value,type);
			return 0;
		}
	}
	return -1;
}

//删除多次请求rw_flag的标签服务
int RemoveMutileTag(CIP * cip,const char* tagname,int rw_flag)
{
	if(cip->MutileTagList == NULL)
	{
		return 0;
	}
	pthread_mutex_lock(&cip->MutileTagList_Mutex);
	TagStruct * ltag ;
	for(ltag = cip->MutileTagList;ltag;ltag=ltag->next)
	{
		if(ltag->rw_flag == rw_flag && tag_strncasecmp(ltag->tag,tagname,128)==0)//存在标签名相同的标签
		{

			if(ltag->prev)	//not the first element
			{
				ltag->prev->next = ltag->next;
			}
			if(ltag->next)	//not  the last element
			{
				ltag->next->prev = ltag->prev;
			}
			if(ltag == cip->MutileTagList)
			{
				cip->MutileTagList=ltag->next;
			}
			ltag->prev = ltag->next = NULL;
			cip_free(ltag);
			pthread_mutex_unlock(&cip->MutileTagList_Mutex);
			return 1;
		}
	}
	pthread_mutex_unlock(&cip->MutileTagList_Mutex);
	return 0;
}

//从MutileTagList获取一个标签
TagStruct *GetTagFromMutileTagList(CIP * cip ,const char * tagname, int rw_flag)
{
	TagStruct * tag ;
	for(tag = cip->MutileTagList;tag;tag=tag->next)
	{
		if(tag->rw_flag == rw_flag && tag_strncasecmp(tag->tag,tagname,128)==0)
		{
			break;
		}
	}
	return tag;
}

//释放标签列表
int ClearMutileTagList(CIP * cip)
{
	TagStruct * tag ;
	TagStruct * rtag ;
	pthread_mutex_lock(&cip->MutileTagList_Mutex);
	for(rtag = cip->MutileTagList;rtag;rtag=tag)
	{
		tag = rtag->next;
		cip_free(rtag);
	}
	cip->MutileTagList=NULL;
	pthread_mutex_unlock(&cip->MutileTagList_Mutex);
	return 0;
}

//发送请求标签值的请求
int SendTagRequstUnitData(CIP * cip)
{
	int item_num=0;
	TagStruct * tag;
	int BytesNum=0;
	int tag_count,count;
	int offset=0;
	EmbeddedRequstStruct *eq;
	if(cip->MutileTagList==NULL && cip->SignleReqTag==NULL)
	{
		return 0;
	}

	if(cip->SignleReqTag)
	{
		pthread_mutex_lock(&cip->SignleTag_Mutex);
		cip->SignleTagFlag = 1;
		tag = cip->SignleReqTag;
		//pthread_mutex_unlock(&cip->SignleTag_Mutex);
		//memcpy(&cip->SignleRepTag,&cip->SignleReqTag,sizeof(TagStruct));
		//pthread_mutex_unlock(&cip->SignleTag_Mutex);
	}else if(cip->LastRequstTag ==NULL)
	{
		tag = cip->MutileTagList;
		pthread_mutex_lock(&cip->MutileTagList_Mutex);
	}else
	{
		tag = cip->LastRequstTag;
	}
	cip->StartRequstTag = tag;
	for( ; tag ;tag = tag->next)
	{
		eq= &cip->DataRequst.ServiceList[item_num];
		if(tag->rw_flag==0)
		{
			eq->Cmd = 0x4C;
			eq->SpecificData = 0x0001;
			tag_count=TagstringToSymbolansiBytes(eq->SymbolTag,tag->tag,sizeof(eq->SymbolTag));
			eq->EpathLength = tag_count/2;

		}else
		{
			eq->Cmd = 0x4D;
			eq->DataType = tag->type;
			memcpy(eq->value,&tag->value,8);
			eq->SpecificData = 0x0001;
			eq->Zero2 = 0;
			tag_count=TagstringToSymbolansiBytes( eq->SymbolTag,tag->tag,sizeof(eq->SymbolTag));
			eq->EpathLength = tag_count/2;
		}
		count=GetEmbeddedRequstStructSizes(eq);
		BytesNum+=count+2;
		if(BytesNum<450 && item_num<128)
		{
			memcpy(&cip->RequstList[item_num],tag,sizeof(TagStruct));
			if(cip->SignleReqTag == NULL){
				cip->LastRequstTag = tag->next;
			}
			item_num++;
		}else
		{
			break;
		}
	}
	if(cip->SignleTagFlag)
	{
		for(int i =0;i<item_num&&cip->SignleReqTag;i++)
		{
			tag = TagStructPop(&cip->SignleReqTag);
			TagStructPush(&cip->SignleRepTag,tag);
		}

	}
	if(item_num==1)
	{
		count=EmbeddedRequstToBytes(&cip->DataRequst.ServiceList[0],cip->CMMRequst.MessageRouterData);
		cip->CMMRequst.DataLength = count+2;
		cip->CMMRequst.ConnetionNum++;
		//ConnectedMessageStructToBytes(&cip->CMMRequst,cip->Send_Buf);
	}
	else if(item_num>1)
	{
		cip->DataRequst.ServiceNum = item_num;
		offset = item_num*2+2;
		for(int i =0;i<item_num;i++)
		{
			cip->DataRequst.ServiceOffset[i]=offset;
			count =EmbeddedRequstToBytes(&cip->DataRequst.ServiceList[i],cip->CMMRequstMessage.RequstData+offset);
			offset+=count;
		}
		cip->CMMRequstMessage.ServiceCode = 0x0A;
		cip->CMMRequstMessage.EpathSize =2;
		cip->CMMRequstMessage.Epath[0] =0x0220;
		cip->CMMRequstMessage.Epath[1] =0x0124;
		cip->CMMRequstMessage.rdata_len=offset;
		memcpy(cip->CMMRequstMessage.RequstData,&cip->DataRequst.ServiceNum,2);
		memcpy(cip->CMMRequstMessage.RequstData+2,cip->DataRequst.ServiceOffset,item_num*2);
		count = MessageRouterRequstToBytes(&cip->CMMRequstMessage,cip->CMMRequst.MessageRouterData);
		cip->CMMRequst.DataLength = count+2;
		cip->CMMRequst.ConnetionNum++;
	}else
	{
		return 0;
	}
	cip->Send_Size = ConnectedMessageStructToBytes(&cip->CMMRequst,cip->Send_Buf);
	return  cip_SendMessage(cip);
}

static int SendRegisterSession(CIP *cip)
{
	ssize_t  size;
	size = write(cip->Handle,&cip->RegSessionRequst,sizeof(RegisterSessionCommandStruct));
	if(size == sizeof(RegisterSessionCommandStruct))
	{
		return 0;
	}
	return -1;
}

/*XXX: cip 客户端处理相关函数*/
//创建新的cip连接，

//c创建一个新的cip客户端， 错误 返回 NULL
CIP * cip_CreatNewClient(const char * destip,uint16_t destport)
{
	CIP * client = cip_malloc(sizeof(CIP));
	strncpy(client->DestIp,destip,sizeof(client->DestIp));
	client->DestPort=destport;
	client->SocketConnectOutMilliseconds=5000;
	client->SocketRcvOutMilliseconds=5000;
	client->mreqMilliseconds=2000;
	client->SignleReqTag = NULL;
	srand(time(0));
	client->T_O_ConnectionID=rand();
	makeheaderstruct(&client->RegSessionRequst.Header);
	client->RegSessionRequst.ProtocolVersion=0x0001;
	client->RegSessionRequst.OptionFlag=0x0000;
	makeheaderstruct(&client->UCMMRequst.Header);
	client->UCMMRequst.Header.Cmd=0x6F;
	client->UCMMRequst.InterfaceHandle=0;
	client->UCMMRequst.TimeOut=0x0001;
	client->UCMMRequst.AddressTypeID=0x0000;
	client->UCMMRequst.AddressLength=0x0000;
	client->UCMMRequst.DataTypeID=0x00B2;
    client->ForwardOpenRequst.Priority = 0x0A;
    client->ForwardOpenRequst.Time_Out_Ticks = 0x05;
    client->ForwardOpenRequst.O_T_ConnectionID = 0x00000000;
    client->ForwardOpenRequst.T_O_ConnectionID = client->T_O_ConnectionID;
    client->ForwardOpenRequst.ConnectionSerialNumber = 0x0000;
    client->ForwardOpenRequst.O_VendorID = 0x0215;
    client->ForwardOpenRequst.O_SerialNumber = client->T_O_ConnectionID;
    client->ForwardOpenRequst.ConnetionTimeOut = 0x01;
    client->ForwardOpenRequst.Resrved1 = 0x00;
    client->ForwardOpenRequst.Resrved2 = 0x00;
    client->ForwardOpenRequst.Resrved3 = 0x00;
    client->ForwardOpenRequst.O_T_RPI = 5000000;
    client->ForwardOpenRequst.O_T_ConnectionParam = 0x43F8;
    client->ForwardOpenRequst.T_O_RPI = 5000000;
    client->ForwardOpenRequst.T_O_ConnectionParam = 0x43F8;
    client->ForwardOpenRequst.TransportType = 0xA3;
    client->ForwardOpenRequst.ConnectionPathSize = 0x03;
    client->ForwardOpenRequst.ConnectionPath[0] = 0x0001;
    client->ForwardOpenRequst.ConnectionPath[1] = 0x0220;
    client->ForwardOpenRequst.ConnectionPath[2] = 0x0124;

	makeheaderstruct(&client->UCMMRequst.Header);
	client->UCMMRequst.Header.Cmd=0x6F;
	client->UCMMRequst.Header.Length=0;
	client->UCMMRequst.InterfaceHandle=0;
	client->UCMMRequst.TimeOut=0x0001;
	client->UCMMRequst.ItemCount = 2;
	client->UCMMRequst.AddressTypeID=0x0000;
	client->UCMMRequst.AddressLength=0x0000;
	client->UCMMRequst.DataTypeID=0x00B2;
	client->UCMMRequstMessage.ServiceCode=0x54;
	client->UCMMRequstMessage.EpathSize=0x02;
	client->UCMMRequstMessage.Epath[0]=0x0620;
	client->UCMMRequstMessage.Epath[1]=0x0124;
	client->UCMMRequstMessage.rdata_len = ForwardOpenRequstStructToBytes(&client->ForwardOpenRequst,client->UCMMRequstMessage.RequstData);
	client->UCMMRequst.DataLength=MessageRouterRequstToBytes(&client->UCMMRequstMessage,client->UCMMRequst.MessageRouterData);
	makeheaderstruct(&client->CMMRequst.Header);
	client->CMMRequst.Header.Cmd=0x70;
	client->CMMRequst.Header.Length=0;
	client->CMMRequst.InterfaceHandle=0;
	client->CMMRequst.TimeOut=0x0001;
	client->CMMRequst.ItemCount = 2;
	client->CMMRequst.AddressTypeID=0x00A1;
	client->CMMRequst.AddressLength=0x0004;
	client->CMMRequst.ConnetionNum=0;
	client->CMMRequst.DataTypeID=0x00B1;

	client->IsConnected = 0;
	client->ConnectState = 0;
	client->connError = 0;
	client->next=NULL;
	client->thread_h = -1;
	CIP_SignleFunctionBind(client,__weak_signle_fptr);
	pthread_mutex_init(&client->MutileTagList_Mutex,NULL);
	pthread_mutex_init(&client->SignleTag_Mutex,NULL);
	pthread_cond_init(&client->SignleTag_Cond,NULL);
	//cip_AddNewClient(client);
	//cip_ThreadStart(client);
	return client;
}

//往 _cip_client_list 添加一个新的客户端，return 0
int cip_AddNewClient(CIP * client)
{
	if(!client) return 0;
	pthread_mutex_lock(&_cip_client_lsit_mutex);
	CIP * cip = _cip_client_list;
	if(_cip_client_list==NULL)
	{
		client->prev = NULL;
		_cip_client_list=client;
	}else
	{
		while(cip->next)
		{
			cip=cip->next;
		}
		cip->next=client;
		client->prev = cip;
	}
	pthread_mutex_unlock(&_cip_client_lsit_mutex);
	return 0;
}

//从 _cip_client_list 移除一个 cip client
int cip_RemoveClient(CIP* client)
{
	if(!client||!client->MutileTagList)return 0;
	pthread_mutex_lock(&_cip_client_lsit_mutex);

	for(CIP*cip = _cip_client_list;cip;cip=cip->next)
	{
		if(cip->Handle == client->Handle)
		{
			if(cip->prev)	//not the first element
			{
				cip->prev->next = cip->next;
			}
			if(cip->next)	//not  the last element
			{
				cip->next->prev = cip->prev;
			}
			if(cip == _cip_client_list)
			{
				_cip_client_list=cip->next;
			}
			cip->prev = cip->next = NULL;
			cip_ClientDisponse(cip);
			pthread_mutex_unlock(&_cip_client_lsit_mutex);
			return 1;
		}
	}
	pthread_mutex_unlock(&_cip_client_lsit_mutex);
	return 0;
}

//在客户端列表中查找指定的客户端
//@return NULL is not find，
CIP * cip_GetClientByIp(const char * destip,uint16_t destport)
{
	pthread_mutex_lock(&_cip_client_lsit_mutex);
	CIP * client ;
	for(client = _cip_client_list;client;client=client->next)
	{
		if(tag_strncasecmp(client->DestIp,destip,sizeof(client->DestIp)==0 && client->DestPort == destport))
		{
			break;
		}
	}
	pthread_mutex_unlock(&_cip_client_lsit_mutex);
	return client;
}
CIP * cip_GetClientByHandle(int handle)
{
	pthread_mutex_lock(&_cip_client_lsit_mutex);
	CIP * client ;
	for(client = _cip_client_list;client;client=client->next)
	{
		if(client->Handle == handle)
		{
			break;
		}
	}
	pthread_mutex_unlock(&_cip_client_lsit_mutex);
	return client;
}

//获取当前连接的句柄最大值
int  cip_GetMaxClientHandle()
{
	int handle = -1;
	pthread_mutex_lock(&_cip_client_lsit_mutex);
	CIP * client ;
	for(client = _cip_client_list;client;client=client->next)
	{
		if(client->Handle > handle)
			handle = client->Handle;
	}
	pthread_mutex_unlock(&_cip_client_lsit_mutex);
	return handle;
}

//清空_cip_client_list
void cip_ClearClientList()
{
	pthread_mutex_lock(&_cip_client_lsit_mutex);
	CIP* cip_next=NULL;
	for(CIP*cip = _cip_client_list;cip;cip=cip_next)
	{
		cip_next=cip->next;
		close(cip->Handle);
		cip_ClientDisponse(cip);
	}
	_cip_client_list=NULL;
	pthread_mutex_unlock(&_cip_client_lsit_mutex);
}

//发送cip客户端发送缓存区内容，return 0 is sucess ，-1 is error
int cip_SendMessage(CIP* cip)
{
	ssize_t size= write(cip->Handle,cip->Send_Buf,cip->Send_Size);
	return size == cip->Send_Size ? 0 : -1 ;
}

//开始建立CIP连接,@return -1 is error
int cip_ClientConnect(CIP * cip)
{
	int res;
	struct timeval tv;
	cip->Handle=socket(AF_INET,SOCK_STREAM,0);
	if(cip->Handle<0)
	{
		DEBUG_PRINTF("what get socket error is [%d] %s\n",errno,strerror(errno));
		return -1;
	}
    memset(&cip->dest_sockaddr, 0, sizeof(cip->dest_sockaddr));
    cip->dest_sockaddr.sin_family = AF_INET;
    cip->dest_sockaddr.sin_addr.s_addr = inet_addr(cip->DestIp);
    cip->dest_sockaddr.sin_port = htons(cip->DestPort);
    cip->ConnectState=1;

    if(cip->SocketConnectOutMilliseconds > 0)
    {
    	tv.tv_sec=cip->SocketConnectOutMilliseconds/1000;
    	tv.tv_usec=(cip->SocketConnectOutMilliseconds%1000)*1000;
    	res=setsockopt(cip->Handle, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));//设置socket连接超时时间
    	//if(res)perror("what has happend ");
    	if(res)DEBUG_PRINTF("what SocketConnectOutMilliseconds error is [%d] %s\n",errno,strerror(errno));
    }
    if(cip->SocketRcvOutMilliseconds>0)
    {
    	tv.tv_sec=cip->SocketRcvOutMilliseconds/1000;
    	tv.tv_usec=(cip->SocketRcvOutMilliseconds%1000)*1000;
    	res=setsockopt(cip->Handle, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));//设置socket连接接收超时时间
    	//if(res)perror("what has happend ");
    	if(res)DEBUG_PRINTF("what SocketRcvOutMilliseconds error is [%d] %s\n",errno,strerror(errno));
    }
    res = connect(cip->Handle,(struct sockaddr *)(&(cip->dest_sockaddr)),sizeof(cip->dest_sockaddr));
    if(res)DEBUG_PRINTF("what connect error is [%d] %s\n",errno,strerror(errno));
    return res;
}

//关闭当前client的tcp连接以及运行线程，并不释放资源
void cip_ClientClose(CIP * cip)
{
	close(cip->Handle);//关闭TCP连接
	cip->ConnectState=0;
	cip->SendState=0;
	cip->IsConnected=0;
	pthread_mutex_init(&cip->SignleTag_Mutex,NULL);
	pthread_mutex_init(&cip->MutileTagList_Mutex,NULL);
	pthread_cond_init(&cip->SignleTag_Cond,NULL);
}

//释放cip client所占用的资源
void cip_ClientDisponse(CIP * cip)
{
	ClearSignleTagList(cip);
	ClearMutileTagList(cip);
	cip_free(cip);
}

//开始cip_client 线程
int cip_ThreadStart(CIP * cip)
{
	int res = pthread_create(&cip->thread_h, NULL, cip_ClientThread, cip);
	if(res!=0)
	{
		cip->thread_h=-1;
		cip->connError=10;
	}else
	{
		cip->ConnectState =1;
		cip->connError=0;
	}
	return res;
}

//停止cip_client 线程
void cip_ThreadStop(CIP * cip)
{
	pthread_cancel(cip->thread_h);
	cip_ClientClose(cip);
}

static void *cip_ClientThread(void *arp)
{
	if(pthread_detach(pthread_self()))
	{
		pthread_exit(NULL);
	}
	if( pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL)!=0)
	{
		pthread_exit(NULL);
	}
	CIP * cip =(CIP *)arp;
	TagStruct *tag;
	EmbeddedResponseStruct *ep;
	//fd_set rdfds;
	struct timeval tv;
	struct timespec tp;
	int len;
	int i;
	int res;
	if(cip==NULL)
	{
		pthread_exit(NULL);
	}
	while(1)
	{
		if(cip->IsConnected)
		{
			if(cip->SendState == 0)//首次发送数据请求
			{
				if(SendTagRequstUnitData(cip)==0)
				{
					cip->SendState=1;
				}else
				{
					cip->connError=2;
					cip_ClientClose(cip);
				}
			}
			else if(cip->SendState == 1)//接收数据处理
			{
				len = read(cip->Handle,cip->Recv_Buf,sizeof(cip->Recv_Buf));
				if(len <0)
				{
					cip_ClientClose( cip);
					cip->connError = 2;
					continue;
				}
				if(len <24)
				{
					continue;
				}
				for(int i = 0; i<len-23;i++)
				{
					HeaderStruct *h=BytesToHeaderStruct(cip->Recv_Buf,i);
					if(h->Cmd != 0x0070 || h->Status !=0 || h->Length > MAX_CIP_MESSAGE_LEN || h->Length > (len-i-24) ||
							h->SessionHandle != cip->CMMRequst.Header.SessionHandle)
					{
						continue;
					}
					if(BytesToConnnetedMessageStruct(&cip->CMMReply,&cip->Recv_Buf[i],len - i)!=0)//无法获取完整的消息结构体
					{
						continue;
					}
					BytesToMessageRouterReponse(&cip->CMMReplyMessage,cip->CMMReply.MessageRouterData,cip->CMMReply.DataLength-2);

					if(cip->CMMReply.MessageRouterData[0] == 0xCC)	//单个读标签响应
					{
						cip->DataReponse.ReponseNum =1 ;
						BytesToEmbeddedResponse(&cip->DataReponse.ReponseList[0],cip->CMMReply.MessageRouterData);
					}
					else if(cip->CMMReply.MessageRouterData[0] == 0xCD)	//单个写标签响应
					{
						cip->DataReponse.ReponseNum =1 ;
						BytesToEmbeddedResponse(&cip->DataReponse.ReponseList[0],cip->CMMReply.MessageRouterData);
					}
					else if(cip->CMMReply.MessageRouterData[0] == 0x8A)	//多重标签响应
					{
						BytesToMultipleServiceReponse(&cip->DataReponse,cip->CMMReplyMessage.ReponseData);
					}
					else
					{
						cip->connError = cip->CMMReplyMessage.Status;
						cip->connError = 2;
						cip_ClientClose(cip);
						break;
					}

					for(tag = cip->StartRequstTag,i=0;tag && i < cip->DataReponse.ReponseNum;i++,tag = tag->next)	//更新请求响应的标签列表
					{
						ep = &cip->DataReponse.ReponseList[i];
						if(ep->StatusCode == 0)
						{
							tag->error = 0;
							if(ep->Cmd == 0xCC)
							{
								tag->type = ep->DataType;
								CipValueCopy(&tag->value,&ep->value,tag->type);
							}
						}
						else
						{
							tag->error =ep->StatusCode;
						}
					}

					if(cip->SignleTagFlag)
					{
						if(cip->SignleReqTag)
						{
							cip->SendState = 1;
							if(SendTagRequstUnitData(cip)!=0)
							{
								cip->connError = 2;
								cip_ClientClose(cip);
							}
							break;
						}
						else
						{
							cip->SignleTagFlag=0;
							pthread_mutex_unlock(&cip->SignleTag_Mutex);
							if(cip->signle_fptr)
							{
								cip->signle_fptr(cip);
							}
							ClearTagList(&cip->SignleRepTag);
						}
					}
					if(cip->LastRequstTag == NULL)
					{
						if(cip->mutile_fptr)
						{
							cip->mutile_fptr(cip);
						}
						pthread_mutex_unlock(&cip->MutileTagList_Mutex);
						cip->SendState =2;
						set_abswaittime(&tp,cip->mreqMilliseconds);
					}
					else if(cip->LastRequstTag != NULL)
					{
						cip->SendState = 1;
						if(SendTagRequstUnitData(cip)!=0)
						{
							cip->connError = 2;
							cip_ClientClose(cip);
						}
					}
					break;
				}
			}
			else if(cip->SendState == 2)//延时等待
			{
				pthread_mutex_lock(&cip->SignleTag_Mutex);
				res=pthread_cond_timedwait(&cip->SignleTag_Cond,&cip->SignleTag_Mutex,&tp);
				pthread_mutex_unlock(&cip->SignleTag_Mutex);
				if(res==ETIMEDOUT)//超时结束，
				{
					if(SendTagRequstUnitData(cip)==0)
					{
						cip->SendState=1;
					}else
					{
						cip->connError = 2;
						cip_ClientClose(cip);
					}
				}
				else if(res == 0)//收到有效的信号量
				{
					set_abswaittime(&tp,cip->mreqMilliseconds);
					if(SendTagRequstUnitData(cip)==0)
					{
						cip->SendState=1;
					}else
					{
						cip->connError = 2;
						cip_ClientClose(cip);
					}
				}
			}
		}else
		{
			if(cip->ConnectState == 0)	//空闲无操作
			{
				tv_MillisecondsSet(&tv,cip->mreqMilliseconds);
				select(0,NULL,NULL,NULL,&tv);
				cip->ConnectState=1;
			}
			else if(cip->ConnectState == 1)	//开始建立 TCP连接
			{
				if(cip_ClientConnect(cip)==0)	//成功创建TCP连接
				{
					if(SendRegisterSession(cip)==0)//成功发送注册会话请求
					{
						cip->ConnectState=2;
					}
				}else
				{
					cip->ConnectState=0;
				}
			}
			else if(cip->ConnectState == 2)//握手第二步，请求建立UCMM连接
			{
				len = read(cip->Handle,cip->Recv_Buf,sizeof(cip->Recv_Buf));
				if(len <0)
				{
					cip->connError = 2;
					cip_ClientClose(cip);
					continue;
				}
				if(len <24)
				{
					continue;
				}
				for(i=0; i<len; i++)
				{
					if(BytesToValue_CIP_UINT(cip->Recv_Buf,i)==0x0065 && strncmp((char*)&cip->Recv_Buf[i+12],CIP_SENDER_CONTEXT,8)==0)
					{
						memcpy(&cip->RegSessionReply,&cip->Recv_Buf[i],sizeof(RegisterSessionCommandStruct));
						cip->UCMMRequst.Header.SessionHandle=cip->RegSessionReply.Header.SessionHandle;
						cip->CMMRequst.Header.SessionHandle=cip->RegSessionReply.Header.SessionHandle;
						cip->Send_Size=UCMMStructToBytes(&cip->UCMMRequst,cip->Send_Buf);
						if(cip->Send_Size>0)
						{
							if(cip_SendMessage(cip)==0)
							{
								cip->ConnectState=3;
								break;
							}
						}
					}
				}
			}
			else if(cip->ConnectState == 3)//握手第三步，建立CMM连接
			{
				len = read(cip->Handle,cip->Recv_Buf,sizeof(cip->Recv_Buf));
				if(len <0)
				{
					cip->connError = 2;
					cip_ClientClose(cip);
					continue;
				}
				if(len <24)
				{
					continue;
				}
				for(int i=0;i<len-23;i++)
				{
					HeaderStruct *h=BytesToHeaderStruct(cip->Recv_Buf,i);
					if(h->Cmd != 0x006F || h->Length > MAX_CIP_MESSAGE_LEN || h->Length > (len-i-24))
					{
						continue;
					}
					if(BytesToUCMMStruct(&cip->UCMMReply,&cip->Recv_Buf[i],len-i)!=0)
					{
						continue;
					}
					if(cip->UCMMReply.DataTypeID != 0x00B2)	//连接类型错误，设置错误代码
					{
						cip->connError = 1;
						cip->ConnectState =0;
						cip_ClientClose(cip);
						break;
					}
					if(BytesToMessageRouterReponse(&cip->UCMMReplyMessage,cip->UCMMReply.MessageRouterData,cip->UCMMReply.DataLength)!=0)
					{
						cip->connError = 1;
						cip->ConnectState =0;
						cip_ClientClose(cip);
						break;
					}
					if(cip->UCMMReplyMessage.RServiceCode != 0xD4)
					{
						cip->connError = 1;
						cip->ConnectState =0;
						cip_ClientClose(cip);
						break;
					}
					if(cip->UCMMReplyMessage.Status == 0)	//连接创建成功
					{
						ForwardOpenResponseStruct *f=BytesToForwardResopnseStruct(cip->UCMMReplyMessage.ReponseData,0);
						cip->CMMRequst.ConnectionID = f->O_T_ConnectionID;
						cip->CMMRequst.ConnetionNum = 0;
						cip->ConnectState=4;
						cip->IsConnected=1;
						cip->SendState=0;
					}else
					{
						cip->ConnectState =0;
						cip->connError = 1;
						cip_ClientClose(cip);
					}
					break;
				}
			}
			else if(cip->ConnectState == 4)		//CMM连接已经建立
			{
				cip->IsConnected=1;
				cip->SendState=0;
			}
		}
	}
}

//static void *select_test(void *arp)
//{
//	fd_set rdfds;
//	struct timeval tv;
//	int fd;
//	FD_ZERO(&rdfds);
//	FD_SET(_cip_client_list->Handle,&rdfds);
//	FD_CLR(_cip_client_list->Handle,&rdfds);
//	//FD_ISSET(_cip_client_list->Handle,&rdfds)
//	rdfds.__fds_bits[(fd / 32)] |= 1UL << (fd % 32);
//	rdfds.__fds_bits[(fd / 32)] &= ~(1UL << (fd % 32));
//	if(((rdfds.__fds_bits[(fd / 32)] & 1UL << (fd % 32)) != 0))
//	{
//
//	}
//
//}


