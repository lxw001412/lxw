/**
 * @file streamOut.h
 * @brief 流输出接口
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-11-29
 * @description 流输出接口
 */

#pragma once

#include <string>
#include <stdint.h>
#include "streamDef.h"

class StreamOut
{
public:
    virtual ~StreamOut() {};

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
    virtual int open(const std::string &id, const std::string &file, AudioCodec_t codec, int channel, int sampleRate, int sampleBits) = 0;

    /**
     * @brief 关闭流
     *
     * @return 0 表示成功，失败时返回其他值
     */
    virtual int close() = 0;

    /**
     * @brief 输出帧
     *
     * @param frameData 帧数据
     *
     * @return 0 表示成功，失败时返回其他值
     * 
     * @notes frameData中的帧数据的有效期为下一次调用read函数前
     */
    virtual int writeData(FrameData_t &frameData) = 0;

    /**
     * @brief 返回音频编码格式
     *
     * @param
     *
     * @return 音频编码格式
     *
     * @notes
     */
    virtual AudioCodec_t audioCodec() = 0;
};
