/**
 * @file streamIn.h
 * @brief 流读取接口
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-11-29
 * @description 流读取接口
 */

#pragma once

#include <string>
#include <stdint.h>
#include "streamDef.h"

class StreamIn
{
public:
    virtual ~StreamIn() {};

    /**
     * @brief 打开流
     * 
     * @param id 流ID
     * @param file 文件路径或流地址
     *
     * @return 0 表示成功，失败时返回其他值
     */
    virtual int open(const std::string &id, const std::string &file) = 0;

    /**
     * @brief 关闭流
     *
     * @return 0 表示成功，失败时返回其他值
     */
    virtual int close() = 0;

    /**
     * @brief 读取流
     *
     * @param data 帧数据
     *
     * @return 0 表示成功，失败时返回其他值
     * 
     * @notes frameData中的帧数据的有效期为下一次调用read函数前
     */
    virtual int readData(FrameData_t &frameData) = 0;

    /**
     * @brief 文件类型流：跳转到指定偏移
     *
     * @param posMs 偏移时长，单位毫秒
     *
     * @return 0 表示成功，失败时返回其他值
     *
     * @notes
     */
    virtual int seekFile(uint64_t posMs) = 0;

    /**
     * @brief 返回当前播放时间点
     *
     * @param 
     *
     * @return 当前播放时间点，单位毫秒
     *
     * @notes
     */
    virtual uint64_t current() = 0;

    /**
     * @brief 返回总时长
     *
     * @param
     *
     * @return 总时长，单位毫秒
     *
     * @notes 非文件流返回0
     */
    virtual uint64_t total() = 0;

    /**
     * @brief 返回采用率
     *
     * @param
     *
     * @return 采用率
     *
     * @notes
     */
    virtual int sampleRate() = 0;

    /**
     * @brief 返回声道数
     *
     * @param
     *
     * @return 声道数
     *
     * @notes
     */
    virtual int channel() = 0;

    /**
     * @brief 比特率
     *
     * @param
     *
     * @return 比特率
     *
     * @notes
     */
    virtual int bitrate() = 0;

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

    /**
     * @brief 返回ffmpeg音频编码格式
     *
     * @param
     *
     * @return 音频编码格式
     *
     * @notes
     */
    virtual int audioAvCodec() = 0;

    /**
     * @brief 暂停
     *
     * @notes
     */
    virtual void pause() = 0;

    /**
     * @brief 恢复
     *
     * @notes
     */
    virtual void resume() = 0;

    /**
     * @brief 等待最后一帧时长
     *
     * @notes
     */
    virtual void lastFrameDelay() = 0;
};

/**
 * @brief ffmpeg库初始化
 *
 * @param 
 *
 * @return 0 表示成功，失败时返回其他值
 */
int ffmpegInit();


/**
 * @brief ffmpeg库释放
 *
 * @param
 *
 * @return 0 表示成功，失败时返回其他值
 */
int ffmpegFini();