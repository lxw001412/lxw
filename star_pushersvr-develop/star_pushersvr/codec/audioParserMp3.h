/**
* @file audioParserMp3.h
* @brief MP3音频帧解析
* @author Bill<pengyouwei@comtom.cn>
* @version 0.1
* @date 2022-10-13
* @description MP3音频帧解析类
*/

#pragma once

#include "audioParser.h"


class Mp3AudioParser : public IAudioParser
{
public:
    Mp3AudioParser();
    virtual ~Mp3AudioParser();
    /**
     * @brief 解析帧数据
     *
     * @param pdata 帧数据指针
     * @param length 帧数据长度
     *
     * @return 0： 解析成功
     *         其它：解析失败
     */
    virtual int ParseFrame(const void* pdata, int length);

    /**
     * @brief 获取当前帧码率
     *
     * @return 码率，单位kbps
     *         -1: 未成功解析帧数据
     *         -2: 不支持获取本属性
     */
    virtual inline int bitRate() const
    {
        if (!m_parseFrameOk)
        {
            return -1;
        }
        return m_bitRate;
    };

    /**
     * @brief 获取当前帧采样率
     *
     * @return 采样率，单位Hz
     *         -1: 未成功解析帧数据
     *         -2: 不支持获取本属性
     */
    virtual inline int sampleRate() const
    {
        if (!m_parseFrameOk)
        {
            return -1;
        }
        return m_sampleRate;
    };

    /**
     * @brief 获取当前帧长度，包含帧头数据
     *
     * @return 帧长度，单位字节
     *         -1: 未成功解析帧数据
     *         -2: 不支持获取本属性
     */
    virtual inline int frameSize() const
    {
        if (!m_parseFrameOk)
        {
            return -1;
        }
        return m_frameSize;
    };

    /**
     * @brief 获取当前帧声道数
     *
     * @return 声道数
     *          1: 单声道
     *          2: 双声道
     *         -1: 未成功解析帧数据
     *         -2: 不支持获取本属性
     */
    virtual inline int channelNum() const
    {
        if (!m_parseFrameOk)
        {
            return -1;
        }
        return m_channelNum;
    };

    /**
     * @brief 获取当前帧时长，单位毫秒
     *
     * @param output: 返回帧时长
     *
     * @return  0: 成功
     *         -1: 未成功解析帧数据
     *         -2: 不支持获取本属性
     */
    virtual inline int duration(double &output) const
    {
        if (!m_parseFrameOk)
        {
            return -1;
        }
        output = m_duration;
        return 0;
    };

private:
    bool m_parseFrameOk;
    int m_bitRate;
    int m_sampleRate;
    int m_frameSize;
    int m_channelNum;
    double m_duration;
};