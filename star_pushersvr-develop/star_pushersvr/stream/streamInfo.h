/**
 * @file streamInfo.h
 * @brief 推流信息类
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-11-30
 * @description 推流信息的存储及获取
 */

#pragma once

#include <string>
#include <vector>
#include <stdint.h>
#include "streamDef.h"

// 文件推流模式
typedef enum _FileMode
{
    FM_SEQ = 1,     // 顺序播放
    FM_RANDOM       // 随机
}FileMode_t;

// 推流状态
typedef enum _StreamStatus
{
    SS_INIT = 0,
    SS_RUNNING,
    SS_PAUSE,
    SS_STOPPED
}StreamStatus_t;

// 文件信息
typedef struct _FileInfo
{
    std::string path;
    std::string id;
    int index;
}FileInfo_t;

// 音频转码参数
typedef struct _AudioConvertArgs
{
    bool enable;
    AudioCodec_t codec;  // 编码类型
    int sampleRate;     // 采样率，单位Hz
    int bitrate;        // 码率，单位kbps
    int channelNum;     // 声道数
}AudioConvertArgs_t;

typedef std::vector<FileInfo_t> FileInfoList_t;

class StreamInfo
{
public:
    virtual ~StreamInfo() {};

    /**
     * @brief 流ID
     */
    virtual void id(const std::string &v) = 0;
    virtual const std::string& id() const = 0;

    /**
     * @brief 推流URL
     */
    virtual void url(const std::string &v) = 0;
    virtual const std::string& url() const = 0;

    /**
     * @brief 是否启用进度回调
     */
    virtual void enableProcessCallback(bool v) = 0;
    virtual bool enableProcessCallback() const = 0;

    /**
     * @brief 实时流--源地址
     */
    virtual void source(const std::string &v) = 0;
    virtual const std::string& source() const = 0;

    /**
     * @brief 文件流--文件列表
     * 
     * @notes 设置文件列表时需按播放顺序排序
     */
    virtual void fileList(const FileInfoList_t &v) = 0;
    virtual const FileInfoList_t& fileList() const = 0;

    /**
     * @brief 持续时长
     */
    virtual void duration(int v) = 0;
    virtual int duration() const = 0;

    /**
     * @brief 文件流-循环次数
     */
    virtual void cycle(int v) = 0;
    virtual int cycle() const = 0;

    /**
     * @brief 文件流-循环模式
     */
    virtual void mode(FileMode_t v) = 0;
    virtual FileMode_t mode() const = 0;

    /**
     * @brief 推流开始时间, UNIX时间戳，单位秒
     */
    virtual void startTime(uint64_t v) = 0;
    virtual uint64_t startTime() const = 0;

    /**
     * @brief 推流结束时间, UNIX时间戳，单位秒
     */
    virtual void stopTime(uint64_t v) = 0;
    virtual uint64_t stopTime() const = 0;

    /**
     * @brief 已播放时长，单位秒
     */
    virtual void playSeconds(int v) = 0;
    virtual int playSeconds() const = 0;

    /**
     * @brief 推流状态
     */
    virtual void status(StreamStatus_t v) = 0;
    virtual StreamStatus_t status() const = 0;

    /**
     * @brief 错误码
     */
    virtual void lastErrorCode(int v) = 0;
    virtual int lastErrorCode() const = 0;

    /**
     * @brief 错误描述
     */
    virtual void lastErrorMsg(const std::string& v) = 0;
    virtual const std::string& lastErrorMsg() const = 0;

    /**
     * @brief 文件推流--当前文件序号
     */
    virtual void curFileIndex(int v) = 0;
    virtual int curFileIndex() const = 0;

    /**
     * @brief 文件推流--当前文件ID
     */
    virtual void curFileId(const std::string& v) = 0;
    virtual const std::string& curFileId() const = 0;

    /**
     * @brief 文件推流--当前文件总时长
     */
    virtual void curFileTotalSeconds(int v) = 0;
    virtual int curFileTotalSeconds() const = 0;

    /**
     * @brief 文件推流--当前文件已播放时长
     */
    virtual void curFilePlaySeconds(int v) = 0;
    virtual int curFilePlaySeconds() const = 0;

    /**
     * @brief 文件流当前-循环次数
     */
    virtual void curCycle(int v) = 0;
    virtual int curCycle() const = 0;

    /**
     * @brief 音频转码参数
     */
    virtual void audioConvertParam(const AudioConvertArgs_t &v) = 0;
    virtual const AudioConvertArgs_t& audioConvertParam() const = 0;

    /**
     * @brief 打印流信息
     */
    virtual const std::string& print(std::string& info) const = 0;

    /**
     * @brief 进度回调URL
     */
    virtual void processCallbackUrl(const std::string& v) = 0;
    virtual const std::string& processCallbackUrl() const = 0;

    /**
     * @brief 是否启用停止回调
     */
    virtual void enableStopCallback(bool v) = 0;
    virtual bool enableStopCallback() const = 0;

    /**
     * @brief 停止回调URL
     */
    virtual void stopCallbackUrl(const std::string& v) = 0;
    virtual const std::string& stopCallbackUrl() const = 0;

    /**
     * @brief 是否启用文件切换回调
     */
    virtual void enableFileStartCallback(bool v) = 0;
    virtual bool enableFileStartCallback() const = 0;

    /**
     * @brief 文件切换回调URL
     */
    virtual void fileStartCallbackUrl(const std::string& v) = 0;
    virtual const std::string& fileStartCallbackUrl() const = 0;

};
