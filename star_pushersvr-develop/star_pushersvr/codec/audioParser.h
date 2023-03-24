/**
 * @file audioParser.h
 * @brief 音频帧解析
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2022-10-13
 * @description 音频帧解析接口类定义
 */

#pragma once

typedef enum
{
    FrameTypeMP3 = 0,
    FrameTypeAAC = 1
} EnumFrameType;

class IAudioParser
{
public:
    static IAudioParser* CreateAudioParser(EnumFrameType type);

    virtual ~IAudioParser() {};
    
    /**
     * @brief 解析帧数据
     *
     * @param pdata 帧数据指针
     * @param length 帧数据长度
     *
     * @return 0： 解析成功
     *         其它：解析失败
     */
    virtual int ParseFrame(const void* pdata, int length) = 0;

    /**
     * @brief 获取当前帧码率
     *
     * @return 码率，单位kbps
     *         -1: 未成功解析帧数据
     *         -2: 不支持获取本属性
     */
    virtual int bitRate() const = 0;
    
    /**
     * @brief 获取当前帧采样率
     *
     * @return 采样率，单位Hz
     *         -1: 未成功解析帧数据
     *         -2: 不支持获取本属性
     */
    virtual int sampleRate() const = 0;

    /**
     * @brief 获取当前帧长度，包含帧头数据
     *
     * @return 帧长度，单位字节
     *         -1: 未成功解析帧数据
     *         -2: 不支持获取本属性
     */
    virtual int frameSize() const = 0;

    /**
     * @brief 获取当前帧声道数
     *
     * @return 声道数
     *          1: 单声道
     *          2: 双声道
     *         -1: 未成功解析帧数据
     *         -2: 不支持获取本属性
     */
    virtual int channelNum() const = 0;
    
    /**
     * @brief 获取当前帧时长，单位毫秒
     *
     * @param output: 返回帧时长
     * 
     * @return  0: 成功
     *         -1: 未成功解析帧数据
     *         -2: 不支持获取本属性
     */
    virtual int duration(double &output) const = 0;
};