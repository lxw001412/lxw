/**
 * @file fileStream.cpp
 * @brief 文件推流类
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-11-30
 * @description 文件推流类
 */
#include "fileStream.h"
#include "spdlogging.h"
#include "tickcount.h"
#include "streamInImp.h"
#include "streamOutFile.h"
#include "streamOutRtmp.h"
#include "AppConfig.h"
#include "Module_Interface.h"
#include "audioParser.h"
#include "autofree.h"
#ifdef ENABLE_RTSP
#include "streamOutRtsp.h"
#endif // ENABLE_RTSP

#include <assert.h>
#include <sstream>
#include <chrono>
#include <json/json.h>

// 音频解码缓存大小
#define AUDIO_DECODE_INIT_BUFFER_SIZE 10240

FileStream::FileStream() :
    StreamBase(),
    m_cmd(NULL),
    m_streamIn(NULL),
    m_streamOut(NULL),
    m_stop(true),
    m_cycle(0),
    m_pauseTime(0),
    m_streamOutErrorTimes(0),
    m_enableReportStop(true),
    m_seekOffset(0)
{
    m_outputBufferSize = AUDIO_DECODE_INIT_BUFFER_SIZE;
    m_outputBuffer = new uint8_t[m_outputBufferSize];

    m_streamReconnectTimes = DEFAULT_STREAM_RECONNECT_TIMES;
    m_streamReconnectIntervalMilliseconds = DEFAULT_STREAM_RECONNECT_INTERVAL_MILLISECONDS;
    AppConfig::instance()->getIntParam("stream", "ReconnectTimes", m_streamReconnectTimes);
    AppConfig::instance()->getIntParam("stream", "ReconnectIntervalMilliseconds", m_streamReconnectIntervalMilliseconds);
}

FileStream::~FileStream()
{
    SPDINFO("[filestream][{}] destruct", m_info.id());
    if (NULL != m_streamIn)
    {
        delete m_streamIn;
        m_streamOut = NULL;
    }
    if (NULL != m_streamOut)
    {
        delete m_streamOut;
        m_streamOut = NULL;
    }
    if (NULL != m_outputBuffer)
    {
        delete []m_outputBuffer;
        m_outputBuffer = NULL;
    }
}

int FileStream::start(const StreamInfo *streamInfo)
{
    assert(streamInfo != NULL);
    std::string info;
    SPDINFO("[filestream][{}] File stream start, info: {}", streamInfo->id(), streamInfo->print(info));

    // 检查参数
    int rc = checkStreamInfo(streamInfo);
    if (0 != rc)
    {
        SPDERROR("[filestream][{}] check stream info error: {}", streamInfo->id(), rc);
        return SEC_PARAM;
    }

    // 创建推流线程
    m_stop = false;
    m_thread = std::thread(&FileStream::svr, this);

    return SEC_OK;
}

int FileStream::stop(bool reportStop)
{
    m_enableReportStop = reportStop;
    if (!m_stop)
    {
        SPDINFO("[filestream][{}] File stream stopping...", m_info.id());
        m_stop = true;
        m_thread.join();
    }
    SPDINFO("[filestream][{}] File stream stopped", m_info.id());
    return SEC_OK;
}

const StreamInfo* FileStream::info()
{
    return &m_info;
}

int FileStream::pause()
{
    SPDINFO("[filestream][{}] File stream puase", m_info.id());
    setCmd(FC_PAUSE, 0, 0);
    return SEC_OK;
}

int FileStream::resume()
{
    SPDINFO("[filestream][{}] File stream resume", m_info.id());
    setCmd(FC_RESUME, 0, 0);
    return SEC_OK;
}

int FileStream::jump(int index, int pos,StreamIn* streamin)
{
    std::string info;
    SPDINFO("[filestream][{}] File stream jump, index: {}, pos: {}", m_info.id(), index, pos);
    if (index >= (int)m_info.fileList().size())
    {
        SPDERROR("[filestream][{}] jump error, index bigger than file size, index: {}, file size:{}", m_info.id(), index, m_info.fileList().size());
        return SEC_PARAM;
    }
    setCmd(FC_JUMP, index, pos, streamin);
    return SEC_OK;
}

void FileStream::setCmd(FS_CMD cmd, int index, int pos,StreamIn* streamin)
{
    m_cmdLock.lock();
    if (m_cmd != NULL)
    {
        delete m_cmd;
    }
    m_cmd = new FileStreamCmd(cmd, index, pos, streamin);
    m_cmdLock.unlock();
}

FileStreamCmd* FileStream::getCmd()
{
    FileStreamCmd* cmd;
    m_cmdLock.lock();
    cmd = m_cmd;
    m_cmd = NULL;
    m_cmdLock.unlock();
    return cmd;
}

int FileStream::initDecoder()
{
    Media_Attr inAttr;
    int ret = 0;

    switch (m_codec)
    {
    case AC_AAC:
        inAttr.mediaId = Coder_AAC;
        inAttr.channals = m_info.audioConvertParam().channelNum;
        ret = m_audioDecoder.open(inAttr);
        break;
    case AC_MP3:
        inAttr.mediaId = Coder_MP3;
        inAttr.channals = m_info.audioConvertParam().channelNum;
        ret = m_audioDecoder.open(inAttr);
        break;
    case AC_MP2:
        inAttr.mediaId = Coder_MP2;
        inAttr.channals = m_info.audioConvertParam().channelNum;
        ret = m_audioDecoder.open(inAttr);
        break;
    case AC_PCM:
        inAttr.mediaId = Coder_PCM;
        inAttr.channals = m_channel;
        inAttr.sampleRate = m_sampleRate;
        ret = m_audioDecoder.open(inAttr);
        break;
    default:
        break;
    }
    return ret;
}

int FileStream::initEncoder()
{
    Media_Attr outAttr;
    int ret = 0;

    outAttr.channals = m_info.audioConvertParam().channelNum;
    outAttr.bitRate = m_info.audioConvertParam().bitrate;
    outAttr.sampleRate = m_info.audioConvertParam().sampleRate;

    if (m_info.audioConvertParam().codec == AC_MP3)
    {
        outAttr.mediaId = Coder_MP3;
        ret = m_audioEncoder.open(outAttr);
    }
    else if (m_info.audioConvertParam().codec == AC_AAC)
    {
        outAttr.mediaId = Coder_AAC;
        ret = m_audioEncoder.open(outAttr);
    }

    return ret;
}

MessageQueue<int>* FileStream::GetMessageQueue()
{
	return &m_playStack;
}

int FileStream::svr()
{
    int rc = 0;
    std::string info;

    m_info.startTime(GetUtcCount());
    setLastError(SEC_OK, "");
    m_info.status(SS_RUNNING);

    // 检查文件列表
    rc = checkFiles();
    if (rc != 0)
    {
        setLastError(SEC_FILELIST, "File list check failed");
        goto end;
    }

    if (m_info.audioConvertParam().enable)
    {
        // 初始化编解码器
        rc = initDecoder();
        if (rc != 0)
        {
            setLastError(SEC_CODEC, "Init decoder failed");
            goto end;
        }
        rc = initEncoder();
        if (rc != 0)
        {
            setLastError(SEC_CODEC, "Init decoder failed");
            goto end;
        }
    }

    // 初始化流输出
    rc = initStreamOut();
    if (rc != 0)
    {
        setLastError(SEC_STREAMOUT, "Stream out init failed");
        goto end;
    }

    //  播放列表循环播放
    while (!m_stop 
        && (m_info.cycle() == 0 || m_cycle < m_info.cycle()) 
        && checkDuration())
    {
        m_info.curCycle(m_cycle);
        SPDINFO("[filestream][{}] File stream cycle: {}", m_info.id(), m_cycle);

        // 生成播放堆栈
        createPlayStack();
        int failedCount = 0;

        // 遍历播放堆栈
        while (!m_stop 
            && m_playStack.size() > 0 
            && checkDuration())
        {
            // 取栈顶文件
            int index;
            m_playStack.pop(index);
            const FileInfo_t& fi = m_info.fileList()[index];

            // 推送文件
            rc = pushFile(fi);
            if (rc != 0)
            {
                goto end;
                failedCount++;
            }
        };

        // 全部失败则停止推流
        if (failedCount >= (int)m_info.fileList().size())
        {
            setLastError(SEC_STREAMIN, "Read files failed");
            goto end;
        }

        // 循环次数加1
        m_cycle++;
    }

end:
    m_info.stopTime(GetUtcCount());
    m_info.status(SS_STOPPED);
    SPDINFO("[filestream][{}] report stop", m_info.id());
    reportStop();
    if (NULL != m_streamOut)
    {
        SPDINFO("[filestream][{}] close stream out", m_info.id());
        m_streamOut->close();
        delete m_streamOut;
        m_streamOut = NULL;
    }
    if (NULL != m_streamIn)
    {
        SPDINFO("[filestream][{}] close stream in", m_info.id());
        m_streamIn->close();
        delete m_streamIn;
        m_streamIn = NULL;
    }
    SPDINFO("[filestream][{}] File stream end: {}", m_info.id(), m_info.print(info));
    return 0;
}


int FileStream::checkStreamInfo(const StreamInfo *streamInfo)
{
    const StreamInfoImp *si = dynamic_cast<const StreamInfoImp*>(streamInfo);
    if (NULL == si)
    {
        SPDERROR("[filestream][{}] dynamic cast error", streamInfo->id());
        return -1;
    }
    m_info = *si;

    // 检查文件列表是否为空
    if (m_info.fileList().size() == 0)
    {
        SPDERROR("[filestream][{}] file list is empty", m_info.id());
        return -2;
    }

    return 0;
}

int FileStream::checkFiles()
{
    StreamInImp streamIn;
    const FileInfoList_t& fl = m_info.fileList();
    int rc = streamIn.open(m_info.id(), fl[0].path);
    if (rc != 0)
    {
        std::stringstream ss;
        ss << "Open " << fl[0].path << " failed";
        setLastError(SEC_STREAMIN, ss.str().c_str());
        return -1;
    }
    // 非文件类型
    if (streamIn.total() == 0)
    {
        std::stringstream ss;
        ss << "File stream " << fl[0].path << " is not file";
        setLastError(SEC_FILELIST, ss.str().c_str());
        return -1;
    }
    m_codec = streamIn.audioCodec();
    m_channel = streamIn.channel();
    m_sampleRate = streamIn.sampleRate();
    streamIn.close();

    //  检查文件列表中文件格式是否一致
    for (unsigned int i = 1; i < fl.size(); i++)
    {
        StreamInImp streamIn;
        rc = streamIn.open(m_info.id(), fl[i].path);
        // 打开失败
        if (rc != 0)
        {
            std::stringstream ss;
            ss << "Open " << fl[0].path << " failed";
            setLastError(SEC_STREAMIN, ss.str().c_str());
            return -1;
        }
        // 非文件类型
        if (streamIn.total() == 0)
        {
            std::stringstream ss;
            ss << "File stream " << fl[i].path << " is not file";
            setLastError(SEC_FILELIST, ss.str().c_str());
            return -1;
        }
        // 格式不一致
         // mp3格式不检查声道和采样率 2022.11.04 PYW
        if (m_codec != streamIn.audioCodec()
            || (m_codec != AC_MP3 && m_channel != streamIn.channel())
            || (m_codec != AC_MP3 && m_sampleRate != streamIn.sampleRate()))
        {
            std::stringstream ss;
            ss << "File list format is diffrent:"
                << " " << fl[0].path << " sr: " << m_sampleRate << " channel: " << m_channel << " codec: " << m_codec
                << " " << fl[i].path << " sr: " << streamIn.sampleRate() << " channel: " << streamIn.channel() << " codec: " << streamIn.audioCodec();
            setLastError(SEC_FILELIST, ss.str().c_str());
            return -1;
        }
        streamIn.close();
    }

    return 0;
}

void FileStream::setLastError(int code, const char* msg)
{
    m_info.lastErrorCode(code);
    m_info.lastErrorMsg(msg);
    if (code != 0)
    {
        SPDERROR("[filestream][{}] stream error: code: {}, msg: {}", m_info.id(), code, msg);
    }
}

int FileStream::initStreamOut()
{
    if (NULL == m_streamOut)
    {
        if (m_info.url().find("rtmp:") == 0)
        {
            m_streamOut = new StreamOutRtmp();
        }
#ifdef ENABLE_RTSP
        else if (m_info.url().find("rtsp:") == 0)
        {
            m_streamOut = new StreamOutRtsp();
        }
#endif // ENABLE_RTSP
    }

    if (NULL == m_streamOut)
    {
        SPDERROR("[filestream][{}] create stream out failed, url: {}", m_info.id(), m_info.url());
        return -1;
    }

    int rc = 0;
    if (m_info.audioConvertParam().enable)
    {
        rc = m_streamOut->open(m_info.id(),
            m_info.url(),
            m_info.audioConvertParam().codec,
            m_info.audioConvertParam().channelNum,
            m_info.audioConvertParam().sampleRate,
            16);
    }
    else
    {
        rc = m_streamOut->open(m_info.id(), 
            m_info.url(), 
            m_codec, 
            m_channel, 
            m_sampleRate, 
            16);
    }
    if (rc != 0)
    {
        delete m_streamOut;
        m_streamOut = NULL;
        SPDERROR("[filestream][{}] init stream out failed: ret: {}, url: {}", m_info.id(), rc, m_info.url());
    }
    return rc;
}

void FileStream::createPlayStack()
{
    while (m_playStack.size() > 0)
    {
        int index;
        m_playStack.pop(index);
    }
    if (m_info.mode() == FM_SEQ)
    {
        for (unsigned int i = 0; i < m_info.fileList().size(); i++)
        {
            m_playStack.push(i);
        }
    }
    else
    {
        // 生成随机序列
        int count = (int)m_info.fileList().size();
        int *array = new int[count];
        for (int i = 0; i < count; i++)
        {
            array[i] = i;
        }
        for (int i = 0; i < count; i++)
        {
            int k = rand() % count;
            if (k == i)
            {
                continue;
            }
            int tmp = array[i];
            array[i] = array[k];
            array[k] = tmp;
        }
        for (int i = 0; i < count; i++)
        {
            m_playStack.push(array[i]);
        }
        delete []array;
    }
}

bool FileStream::checkDuration()
{
    if (m_info.cycle() != 0
        || m_info.duration() == 0)
    {
        // 设置了循环次数或者未设置持续时长
        return true;
    }
    // 经过时长小于持续时长
    return (GetUtcCount() - m_info.startTime() - m_pauseTime) < (unsigned int)m_info.duration();
}

int FileStream::pushFile(const FileInfo_t& fi)
{
    if (NULL == m_streamOut)
    {
        std::stringstream ss;
        ss << "Push data failed, url: " << m_info.url() << ", file: " << m_info.curFileIndex() << ", " << m_info.curFileId();
        setLastError(SEC_STREAMOUT, ss.str().c_str());
        return -2;
    }
    
    if (NULL != m_streamIn)
    {
        delete m_streamIn;
        m_streamIn = NULL;
    }
    m_streamIn = new StreamInImp();
    int rc = m_streamIn->open(m_info.id(), fi.path);
    if (rc != 0)
    {
        std::stringstream ss;
        ss << "Open " << fi.path << " failed";
        setLastError(SEC_STREAMIN, ss.str().c_str());
        return -1;
    }

    SPDINFO("[filestream][{}] Start push file: id:{}, path:{}, index:{}", m_info.id(), fi.id, fi.path, fi.index);
    m_info.status(SS_RUNNING);
    m_info.curFileIndex(fi.index);
    m_info.curFileId(fi.id);
    m_info.curFilePlaySeconds(0);
    m_info.curFileTotalSeconds((int)(m_streamIn->total() / 1000));
    reportFileStart();
    reportProcess();

    bool audioConvert = false;
    if (m_info.audioConvertParam().enable
        && (m_streamIn->audioCodec() != m_info.audioConvertParam().codec
            || m_streamIn->sampleRate() != m_info.audioConvertParam().sampleRate
            || m_streamIn->bitrate() != m_info.audioConvertParam().bitrate
            || m_streamIn->channel() != m_info.audioConvertParam().channelNum))
    {
        audioConvert = true;
    }
    
    FrameData_t fd;
    while (!m_stop
        && checkDuration())
    {
        FileStreamCmd* cmd = getCmd();
        if (cmd != NULL)
        {
            rc = doCmd(cmd);
            if (rc != 0)
            {
                break;
            }
        }
        rc = m_streamIn->readData(fd);
        if (rc != 0)
        {
            // 读取到文件尾  （读取失败问题暂不处理）
            m_streamIn->lastFrameDelay();

            SPDINFO("[filestream][{}] end of file, id:{}, path:{}, index:{}", m_info.id(), fi.id, fi.path, fi.index);
            break;
        }

        std::vector<DATA_STREAM *> output;
        if (audioConvert)
        {
            AVFrame * frame = av_frame_alloc();
            int ret = m_audioDecoder.decode(fd.data, fd.length, 0, frame);
            if (0 != ret)
            {
                SPDERROR("[filestream][{}] decode frame error: {}", m_info.id(), ret);
                continue;
            }

            ret = m_audioEncoder.encode(output, frame);
            av_frame_free(&frame);
            if (0 != ret)
            {
                SPDERROR("[filestream][{}] encode frame error: {}", m_info.id(), ret);
                continue;
            }
            if (output.size() == 0)
            {
                continue;
            }

            for (std::vector<DATA_STREAM *>::iterator it = output.begin(); it != output.end(); ++it)
            {
                DATA_STREAM *outData = *it;
                DataStreamAutoRelease(outData);

                FrameData_t fdOut;
                if (m_info.audioConvertParam().codec == AC_AAC)
                {
                    if (outData->size() <= 7)
                    {
                        continue;
                    }
                    if (outData->size() > m_outputBufferSize)
                    {
                        if (reallocOutputBuff(outData->size()) != 0)
                        {
                            setLastError(SEC_OTHER, "内存分配错误");
                            return -1;
                        }
                    }
                    memcpy(m_outputBuffer, outData->data(), outData->size());
                    IAudioParser *parser = IAudioParser::CreateAudioParser(FrameTypeAAC);
                    AutoFree(IAudioParser, parser);
                    parser->ParseFrame(m_outputBuffer, outData->size());
                    double duration;
                    parser->duration(duration);

                    fdOut.data = m_outputBuffer;
                    fdOut.length = outData->size();
                    fdOut.type = FT_AUDIO;
                    fdOut.duration = int64_t(duration * 1000);
                }
                else if (m_info.audioConvertParam().codec == AC_MP3)
                {
                    if (outData->size() <= 4)
                    {
                        continue;
                    }
                    if (outData->size() > m_outputBufferSize)
                    {
                        if (reallocOutputBuff(outData->size()) != 0)
                        {
                            setLastError(SEC_OTHER, "内存分配错误");
                            return -1;
                        }
                    }
                    memcpy(m_outputBuffer, outData->data(), outData->size());
                    IAudioParser *parser = IAudioParser::CreateAudioParser(FrameTypeMP3);
                    AutoFree(IAudioParser, parser);
                    parser->ParseFrame(m_outputBuffer, outData->size());
                    double duration;
                    parser->duration(duration);

                    fdOut.data = m_outputBuffer;
                    fdOut.length = outData->size();
                    fdOut.type = FT_AUDIO;
                    fdOut.duration = int64_t(duration * 1000);
                }
                else
                {
                    // unsupport codec
                    continue;
                }

                rc = pushFrame(fdOut);
                if (rc != 0)
                {
                    return rc;
                }
            }
        }
        else
        {
            rc = pushFrame(fd);
            if (rc != 0)
            {
                return rc;
            }
        }
        // 更新推流进度
        m_info.curFilePlaySeconds((int)(m_streamIn->current()/1000) + m_seekOffset);
        m_info.playSeconds((int)(GetUtcCount() - m_info.startTime() - m_pauseTime));
        reportProcess();
    }

    return 0;
}

int FileStream::reallocOutputBuff(int size)
{
    if (m_outputBuffer != NULL)
    {
        delete[]m_outputBuffer;
    }
    m_outputBuffer = new uint8_t[size];
    m_outputBufferSize = size;
    if (NULL == m_outputBuffer)
    {
        return 1;
    }
    return 0;
}

int FileStream::pushFrame(FrameData_t &fd)
{
    if (m_streamOut->audioCodec() == AC_AAC && fd.length >= AAC_ADTS_SIZE)
    {
        fd.data = fd.data + AAC_ADTS_SIZE;
        fd.length = fd.length - AAC_ADTS_SIZE;
    }

    int rc = m_streamOut->writeData(fd);
    if (rc != 0)
    {
        m_streamOutErrorTimes++;
        SPDERROR("[filestream][{}], stream out error, url: {}, error times: {}", m_info.id(), m_info.url(), m_streamOutErrorTimes);
        // 暂停读流
        m_streamIn->pause();
        uint64_t pauseTick = GetUtcCount();

        // 推流重连
        for (int i = 0; !m_stop && i < m_streamReconnectTimes; i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(m_streamReconnectIntervalMilliseconds));
            SPDERROR("[filestream][{}], reconnect url: {}", m_info.id(), m_info.url());
            rc = initStreamOut();
            if (0 != rc)
            {
                continue;
            }
            // 推送帧
            rc = m_streamOut->writeData(fd);
            if (0 != rc)
            {
                continue;
            }

            SPDINFO("[filestream][{}], reconnect url ok: {}", m_info.id(), m_info.url());
            break;
        }
        if (0 != rc)
        {
            std::stringstream ss;
            ss << "stream out reconnect failed, url: " << m_info.url() << ", file: " << m_info.curFileIndex() << ", " << m_info.curFileId();
            setLastError(SEC_STREAMOUT, ss.str().c_str());
            return -2;
        }
        // 恢复读流
        m_streamIn->resume();
        m_pauseTime += GetUtcCount() - pauseTick;       // 记录暂停时长
    }
    return rc;
}

int FileStream::doCmd(FileStreamCmd* cmd)
{
    int rc = 0;
    switch (cmd->cmd())
    {
    case FC_PAUSE:
    {
        m_info.status(SS_PAUSE);
        uint64_t start = GetUtcCount();
        SPDINFO("[filestream][{}] pause", m_info.id());
        m_streamIn->pause();
        
        onPause();

        SPDINFO("[filestream][{}] resume", m_info.id());
        m_pauseTime += GetUtcCount() - start;       // 记录暂停时长
        m_info.status(SS_RUNNING);
        m_streamIn->resume();
    }
        break;
    case FC_RESUME:
        break;
    case FC_JUMP:
//      文件调整都重新初始化流输入
        {
		if (!cmd->GetStreamIn())break;
		m_streamIn->close();
		delete m_streamIn;
		m_streamIn = NULL;
		m_streamIn = cmd->GetStreamIn();
		const FileInfo_t& fi = m_info.fileList()[cmd->index()];
            SPDINFO("[filestream][{}] jump to file: id:{}, path:{}, index:{}, pos:{}", 
                m_info.id(), fi.id, fi.path, fi.index, cmd->pos());
            m_info.curFileIndex(fi.index);
            m_info.curFileId(fi.id);
            m_seekOffset = cmd->pos();
            m_info.curFilePlaySeconds(cmd->pos());
            m_info.curFileTotalSeconds((int)(m_streamIn->total() / 1000));
        }
        break;
    default:
        break;
    }
    delete cmd;
    return rc;
}

void FileStream::updatePlayStack(int firstIndex)
{
    if (m_info.mode() == FM_SEQ)
    {
        while (m_playStack.size() > 0)
        {
            int index;
            m_playStack.pop(index);
        }
        for (unsigned int i = firstIndex; i < m_info.fileList().size(); i++)
        {
            m_playStack.push(i);
        }
    }
    else
    {
        // 保留现有随机序列，将指定序号设置到栈顶
        int count = m_playStack.size();
        int *array = new int[count];
        for (int i = 0; i < count; i++)
        {
            int index;
            m_playStack.pop(index);
            array[i] = index;
        }
        m_playStack.push(firstIndex);
        for (int i = 0; i < count; i++)
        {
            if (array[i] == firstIndex)
            {
                continue;
            }
            m_playStack.push(array[i]);
        }
        delete[]array;
    }
}

int FileStream::reportStop()
{
    if (!m_enableReportStop || !m_info.enableStopCallback())
    {
        return 0;
    }
    
    Json::Value msg;
    msg["id"] = m_info.id();
    msg["errorCode"] = m_info.lastErrorCode();
    msg["errorMsg"] = m_info.lastErrorMsg();

    std::string smsg = msg.toStyledString();
    SPDINFO("[filestream][{}] stop callback: {}", m_info.id(), smsg);
    
    // 调用回调模块接口
    HttpRequest *request = HttpRequest::create(m_info.stopCallbackUrl(), "POST");
    request->setParam(smsg, "application/json");
    return Module_Interface::sendStreamStop(request);
}

int FileStream::reportFileStart()
{
    if (!m_info.enableFileStartCallback())
    {
        return 0;
    }
    
    Json::Value msg;
    Json::Value file;

    file["index"] = m_info.curFileIndex();
    file["id"] = m_info.curFileId();

    msg["id"] = m_info.id();
    msg["type"] = "file";
    msg["file"] = file;

    std::string smsg = msg.toStyledString();
    SPDDEBUG("[filestream][{}] file start callback: {}", m_info.id(), smsg);
    
    // 调用回调模块接口
    HttpRequest *request = HttpRequest::create(m_info.fileStartCallbackUrl(), "POST");
    request->setParam(smsg, "application/json");
    return Module_Interface::sendStreamProgress(request);
}

int FileStream::reportProcess()
{
    if (!m_info.enableProcessCallback())
    {
        return 0;
    }

    if (GetUtcCount() - m_lastReportProcessTick < REPORT_PROCESS_INTERVAL)
    {
        return 0;
    }

    Json::Value msg;
    Json::Value file;

    file["index"] = m_info.curFileIndex();
    file["id"] = m_info.curFileId();
    file["pos"] = m_info.curFilePlaySeconds();
    file["total"] = m_info.curFileTotalSeconds();

    msg["id"] = m_info.id();
    msg["type"] = "file";
    if (m_info.status() == SS_PAUSE)
    {
        msg["status"] = "pause";
    }
    else
    {
        msg["status"] = "playing";
    }
    msg["file"] = file;
    msg["playedSeconds"] = m_info.playSeconds();

    std::string smsg = msg.toStyledString();
    SPDDEBUG("[filestream][{}] process callback: {}", m_info.id(), smsg);
    
    // 调用回调模块接口
    HttpRequest *request = HttpRequest::create(m_info.processCallbackUrl(), "POST");
    request->setParam(smsg, "application/json");
    int rc = Module_Interface::sendStreamProgress(request);

    m_lastReportProcessTick = GetUtcCount();
    return rc;
}

StreamIn* FileStream::createSlienceStreamIn(AudioCodec_t codec, int sampleRate, int channel, int sampleBits)
{
    std::string filePath = ".";
    AppConfig::instance()->getStringParam("stream", "silenceFilePath", filePath);
    filePath += "/";
    std::stringstream ss;
    ss << sampleRate << "-" << channel << "-" << sampleBits;
    filePath += ss.str();
    switch (codec)
    {
        case AC_AAC:
            filePath += ".aac";
        break;
        case AC_MP3:
            filePath += ".mp3";
        break;
        case AC_PCM:
            filePath += ".wav";
        break;
        default:
            return NULL;
        break;
    }

    StreamIn* si = new StreamInImp();
    if (NULL == si)
    {
        SPDERROR("[filestream][{}] memory error", m_info.id());
        return NULL;
    }

    int rc = si->open(m_info.id(), filePath);
    if (rc != 0)
    {
        SPDERROR("[filestream][{}] open silence file failed: {}", m_info.id(), filePath);
        delete si;
        return NULL;
    }

    if (si->sampleRate() != sampleRate ||
        si->channel() != channel ||
        si->audioCodec() != codec)
    {
        SPDERROR("[filestream][{}] invalid silence file: {}", m_info.id(), filePath);
        delete si;
        return NULL;
    }
    return si;
}

void FileStream::onPause()
{
    bool resume = false;
    StreamIn* silenceSI = createSlienceStreamIn(m_codec, m_sampleRate, m_channel, 16);

    bool audioConvert = false;
    if (silenceSI != NULL
        && m_info.audioConvertParam().enable
        && (silenceSI->audioCodec() != m_info.audioConvertParam().codec
            || silenceSI->sampleRate() != m_info.audioConvertParam().sampleRate
            || silenceSI->bitrate() != m_info.audioConvertParam().bitrate
            || silenceSI->channel() != m_info.audioConvertParam().channelNum))
    {
        audioConvert = true;
    }

    FrameData_t fd;
    while (!m_stop && !resume)
    {
        FileStreamCmd* c = getCmd();
        if (c != NULL)
        {
            if (c->cmd() == FC_RESUME)
            {
                resume = true;
                delete c;
                break;
            }
            else if (c->cmd() == FC_JUMP)
            {
                doCmd(c);
                resume = true;
                break;
            }
        }

        if (NULL == silenceSI)
        {
            // sleep 100 ms
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else
        {
            int rc = silenceSI->readData(fd);
            if (rc != 0)
            {
                delete silenceSI;
                silenceSI = NULL;
                silenceSI = createSlienceStreamIn(m_codec, m_sampleRate, m_channel, 16);
                continue;
            }

            std::vector<DATA_STREAM *> output;
            if (audioConvert)
            {
                AVFrame * frame = av_frame_alloc();
                int ret = m_audioDecoder.decode(fd.data, fd.length, 0, frame);
                if (0 != ret)
                {
                    SPDERROR("[filestream][{}] decode frame error: {}", m_info.id(), ret);
                    continue;
                }

                ret = m_audioEncoder.encode(output, frame);
                av_frame_free(&frame);
                if (0 != ret)
                {
                    SPDERROR("[filestream][{}] encode frame error: {}", m_info.id(), ret);
                    continue;
                }
                if (output.size() == 0)
                {
                    continue;
                }

                for (std::vector<DATA_STREAM *>::iterator it = output.begin(); it != output.end(); ++it)
                {
                    DATA_STREAM *outData = *it;
                    DataStreamAutoRelease(outData);

                    FrameData_t fdOut;
                    if (m_info.audioConvertParam().codec == AC_AAC)
                    {
                        if (outData->size() <= 7)
                        {
                            continue;
                        }
                        if (outData->size() > m_outputBufferSize)
                        {
                            if (reallocOutputBuff(outData->size()) != 0)
                            {
                                setLastError(SEC_OTHER, "内存分配错误");
                                return;
                            }
                        }
                        memcpy(m_outputBuffer, outData->data(), outData->size());
                        IAudioParser *parser = IAudioParser::CreateAudioParser(FrameTypeAAC);
                        AutoFree(IAudioParser, parser);
                        parser->ParseFrame(m_outputBuffer, outData->size());
                        double duration;
                        parser->duration(duration);

                        fdOut.data = m_outputBuffer;
                        fdOut.length = outData->size();
                        fdOut.type = FT_AUDIO;
                        fdOut.duration = int64_t(duration * 1000);
                    }
                    else if (m_info.audioConvertParam().codec == AC_MP3)
                    {
                        if (outData->size() <= 4)
                        {
                            continue;
                        }
                        if (outData->size() > m_outputBufferSize)
                        {
                            if (reallocOutputBuff(outData->size()) != 0)
                            {
                                setLastError(SEC_OTHER, "内存分配错误");
                                return;
                            }
                        }
                        memcpy(m_outputBuffer, outData->data(), outData->size());
                        IAudioParser *parser = IAudioParser::CreateAudioParser(FrameTypeMP3);
                        AutoFree(IAudioParser, parser);
                        parser->ParseFrame(m_outputBuffer, outData->size());
                        double duration;
                        parser->duration(duration);

                        fdOut.data = m_outputBuffer;
                        fdOut.length = outData->size();
                        fdOut.type = FT_AUDIO;
                        fdOut.duration = int64_t(duration * 1000);
                    }

                    rc = pushFrame(fdOut);
                    if (rc != 0)
                    {
                        break;
                    }
                }
            }
            else
            {
                rc = pushFrame(fd);
                if (rc != 0)
                {
                    break;
                }
            }
            reportProcess();
        }
    }
    if (NULL != silenceSI)
    {
        delete silenceSI;
    }
}