// 
// 创建人： wnet
// 创建时间：Aug 18, 2017
// 功能：test.c
// Copyright (c) 2016 wnet. All Rights Reserved.
// Ver          变更日期        负责人                           变更内容
// ──────────────────────────────────────────────────────────────────────────
// V0.01         Aug 18, 2017                  wnet          初版
// 



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <errno.h>
#include "aes.h"

uint32_t aes_encrypt_string(const char  * key,char *input,char *output)
{
	uint8_t key_buf[16];
	uint8_t* in;
	uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
	uint32_t ilen;
	memset(key_buf,0,16);
	strncpy((char*)key_buf,key,16);
	if((strlen(input)+1)%16==0)
	{
		ilen=strlen(input)+1;
	}else
	{
		ilen=((strlen(input)+1)/16+1)*16;
	}
	in=calloc(ilen,1);
	strncpy((char*)in,input,ilen);
	AES_CBC_encrypt_buffer((uint8_t*)output, (uint8_t*)in, ilen, (const uint8_t*)key_buf, iv);
	free(in);
	return ilen;
}

void aes_decrypt_string(const char  * key,char *input,char *output,uint32_t in_len)
{
	uint8_t key_buf[16];
	uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
	memset(key_buf,0,16);
	strncpy((char*)key_buf,key,16);
	AES_CBC_decrypt_buffer((uint8_t*)output, (uint8_t*)input, in_len, (const uint8_t*)key_buf, iv);
}

int client_connect(char *ip,uint16_t port)
{
	struct timeval otv;
	struct sockaddr_in client_addr;
	char recv_buf[16];
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(port);
	client_addr.sin_addr.s_addr = inet_addr(ip);
	int client_fd = socket(AF_INET,SOCK_STREAM,0);
	if(client_fd<0)
	{
		perror("client socket error");
	}
	otv.tv_sec=2;
	otv.tv_usec=0;
	setsockopt(client_fd,SOL_SOCKET,SO_RCVTIMEO,&otv,sizeof(otv));
	otv.tv_sec=2;
	otv.tv_usec=0;
	setsockopt(client_fd,SOL_SOCKET,SO_SNDTIMEO,&otv,sizeof(otv));
	int res;
	res= connect(client_fd,(struct sockaddr *)&client_addr,sizeof(client_addr));
	if(res <0)
	{
		perror("client connect");
	}
	bzero(recv_buf,16);
	ssize_t len =  read(client_fd,recv_buf,sizeof(recv_buf));
	if(len>0)
	{
		printf("recv is %s\n",recv_buf);
		if(strncmp("MAXS",recv_buf,4)==0)
		{
			return -2;
		}else if(strncmp("OK",recv_buf,2)==0)
		{
			return client_fd;
		}
		return client_fd;
	}else
	{
		return -1;
	}
}

const char *ase_key="wnet_2017_tjhy";
int main(int argc, char* argv[])
{
	char send_msg[256];
	char recv_msg[256];
	char s_buf[256];
	int cid=client_connect("127.0.0.1",5023);
	int err=errno;
	if(cid>=0)
	{
		printf("connected server sucessed id=%d\n",cid);
	}else
	{
		printf("connetced error cid=%d,%s\n",cid,strerror(err));
		return 0;
	}
	snprintf(s_buf,256,"{%d:%d}",1,128);
	int len =aes_encrypt_string(ase_key,s_buf,send_msg);
	int res = write(cid,send_msg,len);
	if(res==len)
	{
		bzero(recv_msg,256);
		len = read(cid,recv_msg,256);
		if(len>0&&strncmp(recv_msg,"OK",2)==0)
		{
			printf("param set ok \n");
		}
	}
	close(cid);
	return 0;
}
