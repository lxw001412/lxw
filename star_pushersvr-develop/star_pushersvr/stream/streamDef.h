/**
 * @file streamDef.h
 * @brief 流相关定义
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-11-29
 * @description 流相关定义
 */

#pragma once

#include <string>
#include <stdint.h>

// 推流类型
typedef enum _StreamType
{
    ST_FILE = 0,
    ST_REALTIME
}StreamType_t;

// 音频编码格式
typedef enum _AudioCodec
{
    AC_AAC = 0,
    AC_MP3,
    AC_PCM,
    AC_MP2,
    AC_UNSUPPORT
}AudioCodec_t;

// 媒体帧类型
typedef enum _FrameType
{
    FT_AUDIO = 0,
    FT_VEDIO
}FrameType_t;

// 媒体帧
typedef struct _FrameData
{
    uint8_t *data;
    int length;
    FrameType_t type;
    int64_t duration;       // 帧时长，单位微秒
}FrameData_t;

// 推流错误码
typedef enum _StreamErrorCode
{
    SEC_OK = 0,
    SEC_PARAM = 100,     // 参数错误

    SEC_FILELIST = 200,     // 文件列表错误
    SEC_STREAMOUT = 201,    // 推流连接错误
    SEC_STREAMIN = 202,     // 打开输入流错误
    SEC_CODEC = 203,        // 编解码器错误
    SEC_OTHER
}StreamErrorCode;

// 缺省流重连次数
#define DEFAULT_STREAM_RECONNECT_TIMES                  3

// 缺省流重连间隔
#define DEFAULT_STREAM_RECONNECT_INTERVAL_MILLISECONDS  3000