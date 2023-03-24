/**
 * @file TypesDefine.h
 * @brief  types define
 * @author K.D, kofera.deng@gmail.com
 * @version 1.0
 * @date 2015-11-11
 */

#ifndef API_PUSHER_TYPES_H
#define API_PUSHER_TYPES_H

#ifdef _WIN32
#ifdef RTMPPUSHER_EXPORTS
#define _API __declspec(dllexport)
#else 
#define _API __declspec(dllimport)
#endif // RTMPPUSHER_EXPORTS
#define _APICALL __stdcall
#else
#define _API
#define _APICALL
#endif

// 支持的音频编码类型定义
#define RTMP_AUDIO_CODEC_AAC			0x00
#define RTMP_AUDIO_CODEC_MP3			0x0E
#define RTMP_AUDIO_CODEC_PCM            0x03

#define RTMP_Pusher_Handler void*

// RTMP推流错误码定义
#ifndef __RTMP_PUSH_MC_ERROR__
#define __RTMP_PUSH_MC_ERROR__
enum 
{
	RTMP_PUSH_MC_NoErr				=	 0,
	RTMP_PUSH_MC_NotEnoughSpace		=	-1,
	RTMP_PUSH_MC_BadURLFormat		=	-2,
	RTMP_PUSH_MC_NotInPushingState	=	-3,
	RTMP_PUSH_MC_NotConn			=	-4,
	RTMP_PUSH_MC_InvalidHandle      =   -5,
	RTMP_PUSH_MC_InvalidParam       =   -6,
	RTMP_PUSH_MC_UnsupportCodec     =   -7,
	RTMP_PUSH_MC_SrtInit            =   -8,
};
typedef  int RTMP_PUSH_MC_Error;
#endif

typedef enum __RTMP_PUSHER_TYPE
{
	RPT_Live = 0,
	RPT_File
} RTMP_PusherType; // PusherType;

/* 推送的媒体帧数据定义 */
typedef struct __RTMP_MEDIA_FRAME_T
{
	unsigned int frameLen;				/* 帧数据长度 */
	unsigned char* frameData;			/* 帧数据 */
	unsigned int timestamp;			/* 时间戳 */
} RTMP_MediaFrame; // MediaFrame;

/* 推送流的媒体属性定义 */
typedef struct __RTMP_MEDIA_INFO_T
{
	unsigned int audioCodec;			/* 音頻編碼类型*/
	unsigned int audioSamplerate;		/* 音頻采样率*/
	unsigned int audioChannel;			/* 音頻通道数*/
	unsigned int audioBits;				/* 音頻通道数*/
} RTMP_MediaInfo; // MediaInfo

typedef enum __RTMP_CB_TYPE_T
{
	CB_TYPE_PushingData = 1,
	CB_TYPE_PushingState
} RTMP_Pusher_CBType;

#define BUFFSIZE 5120

typedef struct __RTMP_PUSHING_DATA_T
{
	char data[BUFFSIZE];				/*媒体幀数据*/
	int  len;							/*数据长度*/
	double duration;					/*数据时长*/
} RTMP_PushingData; // PushingData;

/* 推送回调函数定义 obj 表示用户自定义数据 */
typedef int (*RTMP_PusherCallback)(RTMP_Pusher_CBType type, void *cbdata, void *obj);

#endif
