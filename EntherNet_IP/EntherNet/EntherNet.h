// 
// 创建人： levy
// 创建时间：May 25, 2017
// 功能：EntherNet.h
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         May 25, 2017                  levy          初版
// 


#ifndef ENTHERNET_H_
#define ENTHERNET_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>


#define __DEBUG_MODE 1
#if __DEBUG_MODE
#define DEBUG_PRINTF printf
#else
#define DEBUG_PRINTF(pszFmt,args...)
#endif

#define CIP_TYPE_BOOL 0xC1
#define CIP_TYPE_SINT 0xC2
#define CIP_TYPE_INT 0xC3
#define CIP_TYPE_DINT 0xC4
#define CIP_TYPE_LINT 0xC5
#define CIP_TYPE_USINT 0xC6
#define CIP_TYPE_UINT 0xC7
#define CIP_TYPE_UDINT 0xC8
#define CIP_TYPE_ULINT 0xC9
#define CIP_TYPE_REAL 0xCA

#define STRUCT_ALIGNED __attribute__((packed))

typedef int32_t CIP_BOOL;
typedef int8_t CIP_SINT;
typedef int16_t CIP_INT;
typedef int32_t CIP_DINT;
typedef int64_t CIP_LINT;
typedef uint8_t CIP_USINT;
typedef uint16_t CIP_UINT;
typedef uint32_t CIP_UDINT;
typedef uint64_t CIP_ULINT ;
typedef float CIP_REAL ;
typedef CIP_USINT CIP_BYTE;
typedef CIP_UINT CIP_WORD;

typedef enum
{
	TAG_ERROR_NO=0,			//无错误
	TAG_ERROR_BIT_ERROR=1,	//bit位地址错误
	TAG_ERROR_NOT_EXIST=4,	//标签名不存在
	TAG_ERROR_TYPE_ERROR=5,	//数据类型错误
	TAG_ERROR_NOT_INLIST=10,	//标签列表中没有该标签
	TAG_ERROR_BIT_NUM_ERROR=11,
	TAG_ERROR_OVER_TIME = 12,
	TAG_ERROR_UNKNOWN	=20

}Tag_Error_Enum;


typedef enum
{
	CONN_ERROR_SUCESS=0

}Conn_Error_Enum;


//协议包头结构体 固定24字节
typedef struct
{
	CIP_INT Cmd;
	CIP_INT Length;
	CIP_UDINT SessionHandle;
	CIP_UDINT Status;
	CIP_USINT SenderContext[8];
	CIP_UDINT Options;
} STRUCT_ALIGNED HeaderStruct;

typedef union
{
	char buf[8];
	CIP_BOOL bool;
	CIP_SINT sint;
	CIP_INT iint;
	CIP_DINT dint;
	CIP_LINT lint;
	CIP_USINT usint;
	CIP_UINT uint;
	CIP_UDINT udint;
	CIP_ULINT ulint;
	CIP_REAL real;
	CIP_BYTE byte;
	CIP_WORD word;
}CIP_VALUE;

//SYMBOL_ANSI 数据类型结构体
typedef struct SymbolAnsiStruct
{
	struct SymbolAnsiStruct * next;
	struct SymbolAnsiStruct * prev;
	CIP_USINT SegmentType;    //ID类型通常为0x91，即代表标签名
	CIP_USINT SegmentSize;    //该ID类型的数据字节数
	CIP_USINT SegmentTag[128];   //标签名。如果为奇数，结尾补0
	CIP_USINT IDType;         //标记数据的类型为简单类型，0x28为数组下标,其它的无此选项
	CIP_USINT MemeberID;      //数组的下标编号
} SymbolAnsiStruct;

typedef struct TagStruct
{
	struct TagStruct *next;
	struct TagStruct *prev;
	char tag[128];				//标签名
	CIP_BYTE type;
	CIP_VALUE value;
	int rw_flag;				//0读标签值，1写标签值
	int error;
} TagStruct;


//注册会话结构体
typedef struct
{
	HeaderStruct Header;		//协议包头
	CIP_UINT ProtocolVersion;	//协议版本，当前为1
	CIP_UINT OptionFlag;		//固定为0
}STRUCT_ALIGNED RegisterSessionCommandStruct;

//数据包结构体
typedef struct
{
    HeaderStruct Header;         //CIP包头
    CIP_UDINT InterfaceHandle;   //端口控制句柄为0
    CIP_UINT TimeOut;            //为0
    CIP_UINT ItemCount;          //封装包内的项目数
    int esp_len;
    CIP_BYTE *EncapsulatedPacket;   //封装包的具体内容
} EntherIpDataStruct;

//显式消息数据封装包结构体
typedef struct
{
     HeaderStruct Header;         //CIP包头,包头Cmd为0x70
     CIP_UDINT InterfaceHandle;   //端口控制句柄为0
     CIP_UINT TimeOut;            //请求1，应答为0
     CIP_UINT ItemCount;      //项目数，地址项与数据项共2项
     CIP_UINT AddressTypeID;  //地址类型ID
     CIP_UINT AddressLength;  //地址类的字节数
     CIP_UDINT ConnectionID;  //面向连接的ID,通过ForWardOPen服务获得O_T Network ConnectionID
     CIP_UINT DataTypeID;     //数据类型ID,0xB1表示已经连接的传输
     CIP_UINT DataLength;     //消息路由对象的字节数目
     CIP_UINT ConnetionNum;    //连接序号每次自曾1
     CIP_BYTE MessageRouterData[512];//消息路由对象数据
} ConnectedDataPacketStruct;

//未连接的消息结构体
typedef struct
{
     HeaderStruct Header;         //CIP包头
     CIP_UDINT InterfaceHandle;   //端口控制句柄为0
     CIP_UINT TimeOut;            //应答为0，请求自带超时机构
     CIP_UINT ItemCount;      //项目数，地址项与数据项共2项
     CIP_UINT AddressTypeID;  //地址类型ID，应为0表示NULL
     CIP_UINT AddressLength;  //地址类的16位字长度,应为0
     CIP_UINT DataTypeID;     //数据类型ID,0xB2表示一个UCMM连接
     CIP_UINT DataLength;     //数据的字节数目
     CIP_USINT MessageRouterData[512];//消息路由对象数据
} UnConnectedDataPacketStruct;


//消息路由请求结构体
typedef struct
{
     CIP_USINT ServiceCode;//服务代码
     CIP_USINT EpathSize; //Epath的16位字数目
     CIP_UINT Epath[128];    //Padded Epath代码，属性ID等
     CIP_USINT RequstData[512];  //请求的数据内容
     int rdata_len;
} MessageRouterRequstStrust;
//消息路由响应结构体
typedef struct
{
     CIP_USINT RServiceCode;//回复请求代码
     CIP_USINT Resered;   //必须为0
     CIP_USINT Status;    //应答状态，成功测为 0
     CIP_USINT AddStatusSize; //额外状态的16位字数目
     CIP_UINT AddStatus[128];    //附加的状态
     CIP_USINT ReponseData[512]; //应答的数据内容
     int rdata_len;
} MessageRouterReponseStrust;

//嵌入服务请求数据结构体
typedef struct EmbeddedRequstStruct
{
     CIP_USINT Cmd;          //服务代码0x4C读标签名数据，0x4D，写标签数据
     CIP_USINT EpathLength;    //Epath空间包含的字数
     //SymbolAnsiStruct* SymbolList;
     CIP_BYTE SymbolTag[128];
     CIP_USINT DataType;          //数据类型，读数据无此项
     CIP_USINT Zero2;             //一般为0，读数据无此项
     CIP_UINT SpecificData;    //该服务的特定数据0x0001，
     CIP_BYTE value[8];             //要写入的标签数据的值，读数据无此选项
}EmbeddedRequstStruct;

//嵌入服务响应数据结构体
typedef struct
{
     CIP_USINT Cmd;          //应答服务代码0xCC读标签名数据，0xCD 写响应
     CIP_USINT Zero1;          //必须为0
     CIP_USINT StatusCode;    //状态代码，正确为0
     CIP_USINT AddStatusNum;    //附加状态的16位字数目，通常为0
     CIP_UINT AddStatus[8];   //附加的状态码
     CIP_USINT DataType;         //响应对应标签的数据类型，DINT为0xC4，写响应无此项
     CIP_USINT Zero2;             //一般为0，功能未知，写响应无此项
     CIP_VALUE value;               //读取到的数据，写响应无此项
}EmbeddedResponseStruct;
//多个服务包请求结构体
typedef struct
{
     CIP_UINT ServiceNum; //该包子服务项的数目
     CIP_UINT ServiceOffset[128];    //每一个自服务项的偏移地址
     EmbeddedRequstStruct ServiceList[128];  //子服务列表
}MultipleServiceRequstStruct;
//多个服务包应答结构体
typedef struct
{
     CIP_UINT ReponseNum; //该包子服务项的数目
     CIP_UINT ReponseOffset[128];    //每一个自服务项的偏移地址
     EmbeddedResponseStruct ReponseList[128];  //子服务列表
}MultipleServiceReponseStruct;

//ForwardOpen 服务请求结构体
typedef struct
{
     CIP_BYTE Priority;          //请求超时时间间隔
     CIP_USINT Time_Out_Ticks;    //请求超时计数
     CIP_UDINT O_T_ConnectionID;            //源生产者连接ID
     CIP_UDINT T_O_ConnectionID;            //源消费者连接ID
     CIP_UINT ConnectionSerialNumber;//唯一的16位连接序列号
     CIP_UINT O_VendorID;         //供应商ID,唯一
     CIP_UDINT O_SerialNumber;    //生产商为设备指定的唯一序列号
     CIP_USINT ConnetionTimeOut;  //连接超时倍数，可以为1
     CIP_USINT Resrved1;          //保留的3个字节，为0
     CIP_USINT Resrved2;
     CIP_USINT Resrved3;
     CIP_UDINT O_T_RPI;           //每包之间的请求间隔，单位us
     CIP_WORD O_T_ConnectionParam;//网络连接参数0x48F3,点到点连接，每次传输最大504个字节，优先级紧急
     CIP_UDINT T_O_RPI;           //每包之间的请求间隔，单位us
     CIP_WORD T_O_ConnectionParam;//网络连接参数
     CIP_BYTE TransportType;      //传输触发类型
     CIP_USINT ConnectionPathSize;//连接路径16位字个数， 固定为3
     CIP_UINT ConnectionPath[3];   //连接路径具内容
}STRUCT_ALIGNED ForwardOpenRequstStruct;
//正常ForwardOpen 服务响应结构体
typedef struct
{
     CIP_UDINT O_T_ConnectionID;  //目的消费者连接ID
     CIP_UDINT T_O_ConnectionID;  //目的生产者连接ID
     CIP_UINT ConnectionSerialNumber;//唯一的16位连接序列号
     CIP_UINT O_VendorID;//供应商ID,唯一
     CIP_UDINT O_SerialNumber;//生产商为设备指定的唯一序列号
     CIP_UDINT O_T_RPI;//每包之间的请求间隔，单位us
     CIP_UDINT T_O_RPI;//每包之间的请求间隔，单位us
     CIP_USINT ReplySize;//响应内容大小,该值为0
     CIP_USINT Reserved;  //保留0
    // CIP_BYTE[] Reply;    //具体响应内容数组长度为0
}STRUCT_ALIGNED ForwardOpenResponseStruct;

typedef struct CIP
{
	struct CIP * next;
	struct CIP * prev;
	int Handle;					//TCP连接socket 句柄
	pthread_t thread_h;			//运行线程的句柄
	struct sockaddr_in dest_sockaddr;
	volatile int IsConnected;				//EntherNet连接是否建立
	volatile int ConnectState;				//建立CIP连接的步骤，0未建立，1建立TCP连接，2建立会话，3创建UCMM,连接，4连接已经建立，可以开始传输数据
	volatile int connError;					//创建连接过程中的通信错误
	volatile int SendState;					//UnitData数据发送状态,1正在发送数据，2收到数据应答，0收发未开始
	volatile uint32_t T_O_ConnectionID;		//CIP连接中的连接ID，每次自增
	char DestIp[64];						//目标ip地址
	uint16_t DestPort;						//目标端口号，默认44818
	CIP_BYTE Recv_Buf[1024];				//接收缓存区
	CIP_BYTE Send_Buf[1024];				//接收缓存区
	size_t Send_Size;
	int SocketConnectOutMilliseconds;				//TCP连接最大超时时间，单位ms
	int SocketRcvOutMilliseconds;					//TCP接收最大超时时间
	int mreqMilliseconds;							//标签请求间隔
	int SignleTagFlag;								//指示是否为单次请求的标签
	void (*mutile_fptr)(void * arg);				//完成一次MutileTagrequst执行,该函数执行完成后才会释放资源锁，请误在该函数内更改MutileTagList
	void (*signle_fptr)(void * arg);				//完成单次的标签请求后执行，,请在该函数内释放SignleRepTag占用的资源
    RegisterSessionCommandStruct RegSessionRequst;	//注册会话请求结构体
    RegisterSessionCommandStruct RegSessionReply;	//注册会话应答结构体
    ForwardOpenRequstStruct ForwardOpenRequst;		//ForwardOpen 服务请求结构体
    ForwardOpenResponseStruct ForwardOpenResponse;	//正常ForwardOpen 服务响应结构体
    UnConnectedDataPacketStruct UCMMRequst;			//UCMM请求结构体
    MessageRouterRequstStrust UCMMRequstMessage;	//UCMM消息路由请求结构体
    UnConnectedDataPacketStruct UCMMReply;			//UCMM应答结构体
    MessageRouterReponseStrust UCMMReplyMessage;		//UCMM消息路由应答jiegouti
    ConnectedDataPacketStruct CMMRequst;			//CMM请求结构体
    MessageRouterRequstStrust CMMRequstMessage;		//CMM消息请求结构体
    ConnectedDataPacketStruct CMMReply;				//CMM应答结构体
    MessageRouterReponseStrust CMMReplyMessage;		//CMM消息应答结构体
    MultipleServiceRequstStruct DataRequst;			//多重服务请求结构体
    MultipleServiceReponseStruct DataReponse;		//多重服务应答结构体
    //int(*close)();									//断开当前cip连接
    TagStruct RequstList[128];						//正在处理的标签列表
    TagStruct *MutileTagList;						//需要多次读取的标签列表,
    TagStruct *SignleReqTag;						//单次单个的标签请求，立即请求。
    TagStruct *SignleRepTag;						//单次单个的标签请求的结果
    TagStruct *LastRequstTag;						//上一次处理的最后一个标签的下一个标签
    TagStruct *StartRequstTag;
    pthread_mutex_t MutileTagList_Mutex;			//重复多次请求标签锁
    pthread_mutex_t SignleTag_Mutex;				//单次请求标签锁
    pthread_cond_t SignleTag_Cond;					//

}CIP;

//由字节数组直接读取数据包头结构体
#define BytesToHeaderStruct(buf , StartIndex) ((HeaderStruct*)(buf+StartIndex))
#define BytesToForwardResopnseStruct(buf,StartIndex) ((ForwardOpenResponseStruct*)(buf+StartIndex))
#define BytesToRegisterSessionStruct(buf,StartIndex) ((RegisterSessionCommandStruct*)(buf+StartIndex))
#define BytesToValue_CIP_BOOL(buf,StartIndex) (*((CIP_BOOL*)(buf+StartIndex)))
#define BytesToValue_CIP_SINT(buf,StartIndex) ((CIP_SINT)buf[StartIndex])
#define BytesToValue_CIP_INT(buf,StartIndex) (*((CIP_INT*)(buf+StartIndex)))
#define BytesToValue_CIP_DINT(buf,StartIndex) (*((CIP_DINT*)(buf+StartIndex)))
#define BytesToValue_CIP_LINT(buf,StartIndex) (*((CIP_LINT*)(buf+StartIndex)))
#define BytesToValue_CIP_USINT(buf,StartIndex) ((CIP_USINT)buf[StartIndex])
#define BytesToValue_CIP_UINT(buf,StartIndex) (*((CIP_UINT*)(buf+StartIndex)))
#define BytesToValue_CIP_UDINT(buf,StartIndex) (*((CIP_UDINT*)(buf+StartIndex)))
#define BytesToValue_CIP_ULINT(buf,StartIndex) (*((CIP_ULINT*)(buf+StartIndex)))
#define BytesToValue_CIP_REAL(buf,StartIndex) (*((CIP_REAL*)(buf+StartIndex)))
#define Value_CMP(val1,val2) ((val1)==(val2)?0:((val1)>(val2)?1:-1))
#define CIP_MutileFunctionBind(cip,fptr) (void)((cip)->mutile_fptr=fptr)
#define CIP_SignleFunctionBind(cip,fptr) (void)((cip)->signle_fptr=fptr)

int  ForwardOpenRequstStructToBytes(ForwardOpenRequstStruct *fr, CIP_BYTE * obuf);
int TagstringToSymbolansiBytes(CIP_BYTE * obytes, char * tag,int max_size);
int  EmbeddedRequstToBytes(const EmbeddedRequstStruct *st,CIP_BYTE* obuf);
int GetEmbeddedRequstStructSizes(const EmbeddedRequstStruct *st);
int BytesToEmbeddedResponse(EmbeddedResponseStruct *st ,const CIP_BYTE * bytes);
int MultipleServiceRequstToBytes(const MultipleServiceRequstStruct *st,CIP_BYTE* obuf);
int BytesToMultipleServiceReponse(MultipleServiceReponseStruct *st,const CIP_BYTE * bytes);
int MessageRouterRequstToBytes(const MessageRouterRequstStrust *st, CIP_BYTE * obuf);
int BytesToMessageRouterReponse(MessageRouterReponseStrust * st ,const CIP_BYTE * bytes , int len);
int ConnectedMessageStructToBytes(ConnectedDataPacketStruct *st, CIP_BYTE*obuf);
int BytesToConnnetedMessageStruct(ConnectedDataPacketStruct* st ,const CIP_BYTE * bytes,int len);
int UCMMStructToBytes(UnConnectedDataPacketStruct*st,CIP_BYTE * obuf);
int BytesToUCMMStruct(UnConnectedDataPacketStruct * st , const CIP_BYTE *bytes,int len);


/*TagStruct 处理的相关函数*/
void CipValueCopy(CIP_VALUE *to ,const CIP_VALUE *from,CIP_BYTE type);
char *cip_value2string(char * obuf,CIP_VALUE *val,CIP_BYTE type);
CIP_VALUE * cip_get_value_from_string(CIP_VALUE *val ,const char * str ,CIP_BYTE type);
int get_cip_bit_value(CIP_VALUE* val,CIP_BYTE type, int bitnum);
int set_cip_bit_value(CIP_VALUE* val,CIP_BYTE type, int bitnum,CIP_BOOL bool_val);
int IsRightCIP_Type(CIP_BYTE byte);

int CIPValueCmp(CIP_VALUE *val1 ,const CIP_VALUE *val2,CIP_BYTE type);
TagStruct* CreatTagStruct(const char* tagname , int rw_flag);
TagStruct* CloneTagStruct(const TagStruct *ftag);
void TagStructPush(TagStruct ** ptag ,TagStruct *tag);
TagStruct * TagStructPop(TagStruct ** ptag);
void FreeTagStruct(TagStruct *tag);
TagStruct *GetTagFromListByTag(TagStruct ** taglist ,TagStruct * dtag);
TagStruct *GetTagFromListByTagname(TagStruct ** taglist ,const char * tagname);
void RemoveTagFromTaglistByTag(TagStruct ** taglist ,TagStruct * dtag);
void ClearTagList(TagStruct ** taglist);

/*cip 客户端内部标签处理函数*/
int AddSignleReadTagRequst(CIP*cip,const const char * tagname);
int AddSignleWriteTagRequst(CIP*cip,const const char * tagname,CIP_BYTE type,CIP_VALUE *value);
int AddSignleRequstTaglist(CIP * cip , const TagStruct * list);

int AddMutileReadTagRequst(CIP * cip ,const char * tagname);
int AddMutileWriteTagRequst(CIP * cip ,const char * tagname,CIP_BYTE type,CIP_VALUE *value);
int UpdateMutileTagValue(CIP * cip ,const char * tagname,CIP_BYTE type,CIP_VALUE *value);
TagStruct *GetTagFromMutileTagList(CIP * cip ,const char * tagname, int rw_flag);
int RemoveMutileTag(CIP * cip,const char* tagname,int rw_flag);
TagStruct *GetTagFromMutileTagList(CIP * cip ,const char * tagname, int rw_flag);
int ClearSignleTagList(CIP*cip);
int ClearMutileTagList(CIP * cip);
int SendTagRequstUnitData(CIP * cip);


/*cip 客户端处理函数*/
CIP * cip_CreatNewClient(const char * destip,uint16_t destport);
int cip_AddNewClient(CIP * client);
//如果调用了cip_AddNewClient()请调用cip_RemoveClient释放资源，否则调用cip_ClientDisponse释放资源，适用单客户端程序
int cip_RemoveClient(CIP* client);
void cip_ClientDisponse(CIP * cip);
void cip_ClearClientList();
CIP * cip_GetClientByIp(const char * destip,uint16_t destport);
CIP * cip_GetClientByHandle(int handle);

int cip_ClientConnect(CIP * cip);
void cip_ClientClose(CIP * cip);
int cip_ThreadStart(CIP * cip);
void cip_ThreadStop(CIP * cip);
int cip_SendMessage(CIP* cip);

/*使用说明：
 * 1、首先调用cip_CreatNewClient()创建一个新的客户端，
 * 2、如果有多个客户端请调用cip_AddNewClient(),单客户端不需要该步骤
 * 3、添加需要处理的标签
 * 4、调用cip_ThreadStart()开始连接 EntherNet/IP server
 * 5、期间可以添加删除客户端标签列表中的标签
 * 6、调用cip_ThreadStop()停止客户端连接
 * 7、调用cip_RemoveClient()或cip_ClientDisponse()释放资源
 * 8、
 * */

extern struct timeval tnow,tnow1,tnow2;

#ifdef __cplusplus
}
#endif
#endif /* ENTHERNET_H_ */
