/**
 * @file streamInfoImp.h
 * @brief 推流信息实现类
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-11-30
 * @description 推流信息的存储及获取
 */

#pragma once

#include "streamInfo.h"
#include <json/json.h>

class StreamInfoImp : public StreamInfo
{
public:
    StreamInfoImp() 
    {
        m_audioConvertArgs.enable = false;
    }
    virtual ~StreamInfoImp() {};

    /**
     * @brief 流ID
     */
    virtual void id(const std::string &v) { m_id = v; };
    virtual const std::string& id() const { return m_id; };

    /**
     * @brief 推流URL
     */
    virtual void url(const std::string &v) { m_url = v; };
    virtual const std::string& url() const { return m_url; };

    /**
     * @brief 是否启用进度回调
     */
    virtual void enableProcessCallback(bool v) { m_enableProcessCallback = v; };
    virtual bool enableProcessCallback() const { return m_enableProcessCallback; };

    /**
     * @brief 实时流--源地址
     */
    virtual void source(const std::string &v) { m_source = v; };
    virtual const std::string& source() const { return m_source; };

    /**
     * @brief 文件流--文件列表
     */
    virtual void fileList(const FileInfoList_t &v) { m_fileList = v; };
    virtual const FileInfoList_t& fileList() const { return m_fileList; };

    /**
     * @brief 持续时长
     */
    virtual void duration(int v) { m_duration = v; };
    virtual int duration() const { return m_duration; };

    /**
     * @brief 文件流-循环次数
     */
    virtual void cycle(int v) { m_cycle = v; };
    virtual int cycle() const { return m_cycle; };

    /**
     * @brief 文件流-循环模式:  0: 
     */
    virtual void mode(FileMode_t v) { m_fileMode = v; };
    virtual FileMode_t mode() const { return m_fileMode; };

    /**
     * @brief 推流开始时间, UNIX时间戳，单位秒
     */
    virtual void startTime(uint64_t v) { m_startTime = v; };
    virtual uint64_t startTime() const { return m_startTime; };

    /**
     * @brief 推流结束时间, UNIX时间戳，单位秒
     */
    virtual void stopTime(uint64_t v) { m_stopTime = v; };
    virtual uint64_t stopTime() const { return m_stopTime; };

    /**
     * @brief 已播放时长，单位秒
     */
    virtual void playSeconds(int v) { m_playSeconds = v; };
    virtual int playSeconds() const { return m_playSeconds; };

    /**
     * @brief 推流状态
     */
    virtual void status(StreamStatus_t v) { m_status = v; };
    virtual StreamStatus_t status() const { return m_status; };

    /**
     * @brief 错误码
     */
    virtual void lastErrorCode(int v) { m_lastErrorCode = v; };
    virtual int lastErrorCode() const { return m_lastErrorCode; };

    /**
     * @brief 错误描述
     */
    virtual void lastErrorMsg(const std::string& v) { m_lastErrorMsg = v; };
    virtual const std::string& lastErrorMsg() const { return m_lastErrorMsg; };

    /**
     * @brief 文件推流--当前文件序号
     */
    virtual void curFileIndex(int v) { m_curFileIndex = v; };
    virtual int curFileIndex() const { return m_curFileIndex; };

    /**
     * @brief 文件推流--当前文件ID
     */
    virtual void curFileId(const std::string& v) { m_curFileId = v; };
    virtual const std::string& curFileId() const { return m_curFileId; };

    /**
     * @brief 文件推流--当前文件总时长
     */
    virtual void curFileTotalSeconds(int v) { m_curFileTotalSeconds = v; };
    virtual int curFileTotalSeconds() const { return m_curFileTotalSeconds; };

    /**
     * @brief 文件推流--当前文件已播放时长
     */
    virtual void curFilePlaySeconds(int v) { m_curFilePlaySeconds = v; };
    virtual int curFilePlaySeconds() const { return m_curFilePlaySeconds; };

    /**
     * @brief 文件流当前-循环次数
     */
    virtual void curCycle(int v) { m_curCycle = v; };
    virtual int curCycle() const { return m_curCycle; };

    /**
     * @brief 音频转码参数
     */
    virtual void audioConvertParam(const AudioConvertArgs_t &v) { m_audioConvertArgs = v; };
    virtual const AudioConvertArgs_t& audioConvertParam() const { return m_audioConvertArgs; };

    /**
     * @brief 进度回调URL
     */
    virtual void processCallbackUrl(const std::string& v) { m_processCallbackUrl = v; };
    virtual const std::string& processCallbackUrl() const { return m_processCallbackUrl; };

    /**
     * @brief 是否启用停止回调
     */
    virtual void enableStopCallback(bool v) { m_enableStopCallback = v; };
    virtual bool enableStopCallback() const { return m_enableStopCallback; };

    /**
     * @brief 停止回调URL
     */
    virtual void stopCallbackUrl(const std::string& v) { m_stopCallbackUrl = v; };
    virtual const std::string& stopCallbackUrl() const { return m_stopCallbackUrl; };

    /**
     * @brief 是否启用文件切换回调
     */
    virtual void enableFileStartCallback(bool v) { m_enableFileStartCallback = v; };
    virtual bool enableFileStartCallback() const { return m_enableFileStartCallback; };

    /**
     * @brief 文件切换回调URL
     */
    virtual void fileStartCallbackUrl(const std::string& v) { m_fileStartCallbackUrl = v; };
    virtual const std::string& fileStartCallbackUrl() const { return m_fileStartCallbackUrl; };

    /**
     * @brief 打印流信息
     */
    virtual const std::string& print(std::string& info) const {
        Json::Value jdata;
        jdata["id"] = m_id;
        jdata["url"] = m_url;
        jdata["enableCallback"] = m_enableProcessCallback;
        jdata["processCbUrl"] = m_processCallbackUrl;
        jdata["enableFileStartCb"] = m_enableFileStartCallback;
        jdata["fileStarCbUrl"] = m_fileStartCallbackUrl;
        jdata["enableStopCb"] = m_enableStopCallback;
        jdata["stopCbUrl"] = m_stopCallbackUrl;
        jdata["source"] = m_source;
        for (FileInfoList_t::const_iterator it = m_fileList.begin(); it != m_fileList.end(); ++it)
        {
            Json::Value jfile;
            jfile["path"] = it->path;
            jfile["id"] = it->id;
            jfile["index"] = it->index;
            jdata["files"].append(jfile);
        }
        jdata["duration"] = m_duration;
        jdata["cycle"] = m_cycle;
        jdata["fileMode"] = m_fileMode;
        jdata["startTime"] = m_startTime;
        jdata["stopTime"] = m_stopTime;
        jdata["playSeconds"] = m_playSeconds;
        jdata["status"] = m_status;
        jdata["lastErrorCode"] = m_lastErrorCode;
        jdata["lastErrorMsg"] = m_lastErrorMsg;
        jdata["curFileIndex"] = m_curFileIndex;
        jdata["curFileId"] = m_curFileId;
        jdata["curFileTotalSeconds"] = m_curFileTotalSeconds;
        jdata["curFilePlaySeconds"] = m_curFilePlaySeconds;
        jdata["curCycle"] = m_curCycle;
        Json::Value jAudioConvertArgs;
        jAudioConvertArgs["enable"] = m_audioConvertArgs.enable;
        jAudioConvertArgs["codec"] = m_audioConvertArgs.codec;
        jAudioConvertArgs["ar"] = m_audioConvertArgs.sampleRate;
        jAudioConvertArgs["ab"] = m_audioConvertArgs.bitrate;
        jAudioConvertArgs["ac"] = m_audioConvertArgs.channelNum;
        jdata["audioConvert"] = jAudioConvertArgs;
        info = jdata.toStyledString();
        return info;
    };
    
 private:
    std::string m_id;
    std::string m_url;
    bool m_enableProcessCallback = false;
    std::string m_source;
    FileInfoList_t m_fileList;
    int m_duration = 0;
    int m_cycle = 0;
    FileMode_t m_fileMode = FM_SEQ;
    uint64_t m_startTime = 0;
    uint64_t m_stopTime = 0;
    int m_playSeconds = 0;
    StreamStatus_t m_status = SS_INIT;
    int m_lastErrorCode = 0;
    std::string m_lastErrorMsg;
    int m_curFileIndex = 0;
    std::string m_curFileId;
    int m_curFileTotalSeconds = 0;
    int m_curFilePlaySeconds = 0;
    int m_curCycle = 0;
    AudioConvertArgs_t m_audioConvertArgs;
    std::string m_processCallbackUrl;
    bool m_enableStopCallback = true;
    std::string m_stopCallbackUrl;
    bool m_enableFileStartCallback = false;
    std::string m_fileStartCallbackUrl;
};
