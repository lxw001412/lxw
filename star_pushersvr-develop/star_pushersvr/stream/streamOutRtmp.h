/**
 * @file streamOutRTMP.h
 * @brief RTMP流输出类
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-11-29
 * @description RTMP流输出类
 */

#pragma once

#include "streamOut.h"
#include "API_PusherTypes.h"
#include "API_PusherModule.h"
#include <string>

class StreamOutRtmp : public StreamOut
{
public:
    StreamOutRtmp();

    virtual ~StreamOutRtmp();

    /**
     * @brief 打开流
     * 
     * @param id 流ID
     * @param file 文件路径或流地址
     * @param codec 编码格式
     * @param channel 声道数
     * @param sampleRate 采样率
     * @param sampleBits 采样位数
     *
     * @return 0 表示成功，失败时返回其他值
     */
    virtual int open(const std::string &id, const std::string &file, AudioCodec_t codec, int channel, int sampleRate, int sampleBits);

    /**
     * @brief 关闭流
     *
     * @return 0 表示成功，失败时返回其他值
     */
    virtual int close();

    /**
     * @brief 输出帧
     *
     * @param frameData 帧数据
     *
     * @return 0 表示成功，失败时返回其他值
     * 
     * @notes frameData中的帧数据的有效期为下一次调用read函数前
     */
    virtual int writeData(FrameData_t &frameData);

    /**
     * @brief 返回音频编码格式
     *
     * @param
     *
     * @return 音频编码格式
     *
     * @notes
     */
    virtual AudioCodec_t audioCodec() {
        return m_codec;
    };

protected:
    static int callback(RTMP_Pusher_CBType type, void *cbdata, void *obj);
    int handleCallback(RTMP_Pusher_CBType type, void *cbdata);

private:
    RTMP_Pusher_Handler m_handler;
    RTMP_MediaInfo m_mi;
    std::string m_url;
    int64_t m_timestamp;
    uint64_t m_lastFrameTick;
    uint64_t m_startTick;
    std::string m_id;
    AudioCodec_t m_codec;
};
