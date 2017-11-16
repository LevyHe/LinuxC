 /*
* Copyright(C) 2011,Hikvision Digital Technology Co., Ltd 
* 
* File   name��main.cpp
* Discription��demo for muti thread get stream
* Version    ��1.0
* Author     ��luoyuhua
* Create Date��2011-12-10
* Modification History��
*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "HCNetSDK.h"

#ifdef __cplusplus
extern "C" {  // 即使这是一个C++程序，下列这个函数的实现也要以C约定的风格来搞！
#endif

static NET_DVR_DEVICEINFO_V30 struDeviceInfo;
static int sChanel=1;

int Net_DVR_Login(char *szHost,unsigned short sPort,char *szUser ,char *szPasswd )
{
	int iUserID;
	iUserID=NET_DVR_Login_V30(szHost,sPort,szUser,szPasswd,&struDeviceInfo);
	return iUserID;

}
BOOL Net_DVR_LogInit(int level,char *recordfile)
{
	if(NET_DVR_Init())
		return NET_DVR_SetLogToFile(level, recordfile);
	else
		return FALSE;
}

int Net_DVR_SetsChanel(int ichanel)
{
	sChanel=ichanel;
	return sChanel;
}
void Net_DVR_CleanLogout(int iUserID)
{
	NET_DVR_Logout(iUserID);
	NET_DVR_Cleanup();
}

/*功能：从摄像头获取一张图片保存到jpg中，size为图片大小
 * @return 0 is sucess, -1 error
 * */
int Net_getimage(int iuserID,char * jpg,unsigned int max_size,unsigned int *size)
{

	NET_DVR_JPEGPARA lpJpegPara;
	lpJpegPara.wPicSize=5  ;// #-HD720P(1280*720)
	lpJpegPara.wPicQuality=0  ;
	DWORD lpSizeReturned;
	if(NET_DVR_CaptureJPEGPicture_NEW(iuserID,sChanel,&lpJpegPara,jpg,max_size,size)==TRUE)
	{
		return 0;
	}else
	{
		return -1;
	}
}
/*功能：NetVideoPlayStart
 * @iuserid： Net_login（）返回值
 * @return：-1 error，iRealPlayHandle>0
 * */
int NetVideoPlayStart(int iuserid)
{

  NET_DVR_PREVIEWINFO struPreviewInfo;
  struPreviewInfo.lChannel =sChanel;
  struPreviewInfo.dwStreamType = 0;
  struPreviewInfo.dwLinkMode = 0;
  struPreviewInfo.bBlocked = 1;
  struPreviewInfo.bPassbackRecord  = 1;
  LONG iRealPlayHandle=NET_DVR_RealPlay_V40(iuserid, (&struPreviewInfo), NULL, NULL);
  return iRealPlayHandle;
}
/*功能：NetVideoPlayStop
 * @iuserid： Net_login（）返回值
 * */
void NetVideoPlayStop(int lhandle)
{
	NET_DVR_StopRealPlay(lhandle);
}

/*功能：Net_video_start_record
 * @iuserid： NetVideoPlayStart（）返回值
 * @return：-1 error，0 sucess
 * */
int Net_video_start_record(int lhandle,char*szVideoFile )
{

	if(NET_DVR_SaveRealData(lhandle,szVideoFile)==FALSE)
	{
		return -1;
	}
	return 0;
}
/*Net_video_stop_record*/
void Net_video_stop_record(int lhandle)
{
	NET_DVR_StopSaveRealData(lhandle);
}

unsigned int Net_DVR_LastError()
{
	return NET_DVR_GetLastError();
}


#ifdef __cplusplus
}
#endif


