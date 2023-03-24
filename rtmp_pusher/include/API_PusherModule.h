/**
 * @file API_PusherModule.h
 * @brief  Pusher Module API
 * @author K.D, kofera.deng@gmail.com
 * @version 1.0
 * @date 2015-11-13
 */

#ifndef API_PUSHER_MODULE_H
#define API_PUSHER_MODULE_H

#include "./API_PusherTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif 

	/**
	 * @brief  RTMP_Pusher_Create 
	 *		创建推送流句柄
	 * @return  NULL or 推送流句柄 
	 */
	_API RTMP_Pusher_Handler _APICALL RTMP_Pusher_Create();


	/**
	 * @brief  RTMP_Pusher_SetCallback 
	 *		推送流回调函数,　根据推送状态触发
	 * @param handler	推送流句柄
	 * @param cb		注册的回调函数
	 *
	 * @return  返回处理结果 
	 */
	_API int _APICALL RTMP_Pusher_SetCallback(RTMP_Pusher_Handler handler, RTMP_PusherCallback cb, void* cbParam);

	/**
	 * @brief  RTMP_Pusher_SetCallback 
	 *		推送流回调函数,　根据推送状态触发
	 * @param handler	推送流句柄
	 * @param cb		注册的回调函数
	 *
	 * @return  返回处理结果 
	 */
	_API void* _APICALL RTMP_Pusher_GetCallback(RTMP_Pusher_Handler handler);

	/**
	 * @brief  RTMP_Pusher_StartStream 
	 *		开始推送流
	 * @param handler	推送流句柄
	 * @param url		推送流RTMP URL
	 * @param mi		推送流媒体信息
	 * @param type		推送流媒体类型
	 *
	 * @return			返回处理结果 
	 */
    _API int _APICALL RTMP_Pusher_StartStream(RTMP_Pusher_Handler handler, const char* url, const RTMP_MediaInfo* mi, RTMP_PusherType type);

	/**
	 * @brief  RTMP_Pusher_CloseStream 
	 *		结束推送流
	 * @param handler　推送流句柄
	 *
	 * @return   
	 */
	_API void _APICALL RTMP_Pusher_CloseStream(RTMP_Pusher_Handler handler);

	/**
	 * @brief  RTMP_Pusher_Release 
	 *		推送流数据资源释放
	 * @param handler	推送流句柄
	 *
	 * @return  
	 */
	_API void _APICALL RTMP_Pusher_Release(RTMP_Pusher_Handler handler);


	/**
	 * @brief  RTMP_Pusher_PushFrame 
	 *		推送媒体数据帧
	 * @param handler	推送流句柄
	 * @param frame		媒体数据帧
	 *
	 * @return  返回处理结果 
	 */
	_API int _APICALL RTMP_Pusher_PushFrame(RTMP_Pusher_Handler handler, RTMP_MediaFrame* frame);
#ifdef __cplusplus
}
#endif 

#endif 
