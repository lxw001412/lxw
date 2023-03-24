/**
 * @file streamInImp.cpp
 * @brief 流读取实现类
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-11-29
 * @description 流读取实现类
 */

#include "streamInImp.h"
#include "tickcount.h"
#include "spdlogging.h"
#include "AppConfig.h"

// 输入流超时时长，10000毫秒
#define STREAM_IN_TIMEOUT 10000

int ffmpegInit()
{
    avformat_network_init();
    return 0;
}

int ffmpegFini()
{
    return 0;
}

StreamInImp::StreamInImp() : StreamIn(),
m_adts(NULL),
m_ifmtCtx(NULL),
m_audioIndex(0),
m_videoIndex(0),
m_duration(0),
m_starttime(0),
m_lastFrameTick(0),
m_audioCurrent(0),
m_stop(false),
m_sp(SP_FILE),
m_avctx(NULL),
m_audioCodec(AC_UNSUPPORT),
m_audioAvCodec(0),
m_bitrate(0),
m_pauseTime(0),
m_pauseTick(0),
m_seekedFile(false),
m_startPtsTime(-1),
m_interruptStartTickCount(0)
{
    m_frameBufferLength = 1600;
    m_frameBuffer = new uint8_t[m_frameBufferLength];
}

StreamInImp::~StreamInImp()
{
    SPDINFO("[stream-in][{}] destruct, file: {}", m_id, m_file);
    close();
    if (NULL != m_adts)
    {
        delete m_adts;
        m_adts = NULL;
    }
    if (NULL != m_avctx)
    {
        avcodec_free_context(&m_avctx);
    }
    delete []m_frameBuffer;
    m_frameBuffer = NULL;
}


int StreamInImp::open(const std::string &id, const std::string &file)
{
    m_id = id;
    m_startPtsTime = -1;

    SPDINFO("[stream-in][{}] opening {}", m_id, file);

    char buf[AV_ERROR_MAX_STRING_SIZE + 1] = { 0 };
    m_file = file;
    int ret = 0;

    m_streamInTimeout = STREAM_IN_TIMEOUT;
    int tempParam = 0;
    if (0 == AppConfig::instance()->getIntParam("stream", "streamInTimeoutMs", tempParam))
    {
        m_streamInTimeout = tempParam;
    }

    AVDictionary * avdic = NULL;
    if (file.find("rtmp:") == 0)
    {
        m_ifmtCtx = avformat_alloc_context();
        m_ifmtCtx->interrupt_callback.callback = &StreamInImp::statiCallBack;
        m_ifmtCtx->interrupt_callback.opaque = this;
        av_dict_set(&avdic, "probesize", "2048", 0);
        m_sp = SP_RTMP;
    }
    else if (file.find("rtp:") == 0)
    {
        m_ifmtCtx = avformat_alloc_context();
        m_ifmtCtx->interrupt_callback.callback = &StreamInImp::statiCallBack;
        m_ifmtCtx->interrupt_callback.opaque = this;
        m_sp = SP_RTP;
    }
    else if (file.find("http:") == 0 || file.find("https:") == 0)
    {
        m_ifmtCtx = avformat_alloc_context();
        m_ifmtCtx->interrupt_callback.callback = &StreamInImp::statiCallBack;
        m_ifmtCtx->interrupt_callback.opaque = this;
        av_dict_set(&avdic, "recv_buffer_size", "65535", 0);    // 设置TCP接收缓存大小，避免系统默认缓存太大文件传输提前完成后服务端断开TCP连接 PYW 2022.11.23
        av_dict_set(&avdic, "reconnect", "1", 0);               // 设置HTTP断线自动重连 PYW 2023.03.09

        m_sp = SP_HTTP;
    }
    else if (file.find("rtsp:") == 0)
    {
        av_dict_set(&avdic, "rtsp_transport", "tcp", 0);
        av_dict_set(&avdic, "stimeout", "5", 0);
        m_sp = SP_RTSP;
    }
    else if (file.find("udp:") == 0)
    {
        //等待1s, 单位微秒
        av_dict_set(&avdic, "timeout", "1000000", 0);
        m_sp = SP_UDP;
    }
    else
    {
        m_sp = SP_FILE;
    }

    m_interruptStartTickCount = GetMillisecondCounter();
    if ((ret = avformat_open_input(&m_ifmtCtx, file.c_str(), 0, &avdic)) != 0)
    {
        m_lastError = av_make_error_string(buf, AV_ERROR_MAX_STRING_SIZE, ret);
        ret = -1;
        goto error;
    }

    if ((ret = avformat_find_stream_info(m_ifmtCtx, 0)) < 0)
    {
        m_lastError = av_make_error_string(buf, AV_ERROR_MAX_STRING_SIZE, ret);
        ret = -2;
        goto error;
    }

    for (unsigned int i = 0; i < m_ifmtCtx->nb_streams; i++)
    {
        switch (m_ifmtCtx->streams[i]->codecpar->codec_type)
        {
        case AVMEDIA_TYPE_AUDIO:
            m_audioIndex = i;
            m_audioAvCodec = m_ifmtCtx->streams[i]->codecpar->codec_id;
            switch (m_ifmtCtx->streams[i]->codecpar->codec_id)
            {
            case  AV_CODEC_ID_AAC:
                m_audioCodec = AC_AAC;
                if (m_adts != NULL)
                {
                    delete m_adts;
                }
                m_adts = new AAC_ADTS;
                m_adts->sample_rate_index(m_ifmtCtx->streams[i]->codecpar->sample_rate);
                m_adts->channel_configuration(m_ifmtCtx->streams[i]->codecpar->channels);
                break;
            case  AV_CODEC_ID_MP3:
                m_audioCodec = AC_MP3;
                break;
            case  AV_CODEC_ID_MP2:
                m_audioCodec = AC_MP2;
                break;
            case AV_CODEC_ID_PCM_S16LE:
                m_audioCodec = AC_PCM;
                break;
            default:
                m_audioCodec = AC_UNSUPPORT;
                break;
            }
            m_sampleRate = m_ifmtCtx->streams[i]->codecpar->sample_rate;
            m_channel = m_ifmtCtx->streams[i]->codecpar->channels;
            m_bitrate = (int)m_ifmtCtx->streams[i]->codecpar->bit_rate;
            if (m_adts != NULL)
            {
                m_duration = (int64_t)(m_adts->frame_duration() * 1000);
            }
            else
            {
                m_duration = m_ifmtCtx->duration;
            }
            break;
        default:
            SPDINFO("[stream-in][{}] Unused sub stream {} in {}", 
                m_id, m_ifmtCtx->streams[i]->codecpar->codec_type, file);
            break;
        }
    }
    SPDINFO("[stream-in][{}] opened: file: {}, codec: {}, sample rate: {}, channels: {}, duration: {}ms",
        m_id,
        file,
        m_audioCodec,
        m_sampleRate,
        m_channel,
        m_duration/1000);

    av_dump_format(m_ifmtCtx, 0, file.c_str(), 0);
    m_starttime = av_gettime();

    return ret;

error:
    SPDERROR("[stream-in][{}] open {} failed, error: {}, ret: {}", m_id, file, m_lastError, ret);
    if (NULL != m_ifmtCtx)
    {
        avformat_close_input(&m_ifmtCtx);
    }
    return ret;
}

int StreamInImp::close()
{
    SPDINFO("[stream-in][{}] close {}", m_id, m_file);
    if (NULL != m_ifmtCtx)
    {
        avformat_close_input(&m_ifmtCtx);
        avformat_free_context(m_ifmtCtx);
    }
    return 0;
}

int StreamInImp::readData(FrameData_t &frameData)
{
    AVPacket *pkt = NULL;
    int rc = 0;

    do
    {
        pkt = av_packet_alloc();
        rc = readAvPacket(pkt);
        if (rc < 0)
        {
            av_packet_unref(pkt);
            av_packet_free(&pkt);
            return -1;
        }
        if (pkt->stream_index == m_audioIndex)
        {
            break;
        }
        av_packet_unref(pkt);
        av_packet_free(&pkt);
        if (m_stop)
        {
            return -1;
        }
    } while (true);

    // 控制流速率
    audiotime(pkt);

    // 检查AAC帧是否需要增加ADTS头，并且设置帧长度
    bool addADTS = false;
    if (AC_AAC == m_audioCodec &&
        (SP_RTMP == m_sp
            || SP_RTSP == m_sp
            || !(pkt->data[0] == 0xff && ((pkt->data[1] >> 4) == 0x0F))))
    {
        frameData.length = pkt->size + AAC_ADTS_SIZE;
        m_adts->aac_frame_length(pkt->size + AAC_ADTS_SIZE);
        addADTS = true;
    }
    else
    {
        frameData.length = pkt->size;
    }

    // 检查帧缓存是否足够大
    if (frameData.length > m_frameBufferLength)
    {
        m_frameBufferLength = frameData.length + 256;
        delete []m_frameBuffer;
        m_frameBuffer = new uint8_t[m_frameBufferLength];
    }

    // 拷贝帧数据
    int pos = 0;
    if (addADTS)
    {
        memcpy(m_frameBuffer, m_adts->Data(), AAC_ADTS_SIZE);
        pos += AAC_ADTS_SIZE;
    }
    memcpy(m_frameBuffer + pos, pkt->data, pkt->size);
    frameData.data = m_frameBuffer;

    // 帧时长
    if (addADTS && m_adts != NULL)
    {
        frameData.duration = (int64_t)(m_adts->frame_duration() * 1000);
    }
    else
    {
        frameData.duration = m_frameDuration;
    }

    // 设置帧类型
    frameData.type = FT_AUDIO;

    // 释放AVPacket内存
    av_packet_unref(pkt);
    av_packet_free(&pkt);
    return 0;
}

int StreamInImp::seekFile(uint64_t posMs)
{
    SPDINFO("[stream-in][{}] seeking file, pos-ms: {}, duration: {}", m_id, posMs, m_duration);
    if (NULL == m_ifmtCtx || posMs < 0 || 0 == m_duration)
    {
        SPDERROR("[stream-in][{}] seek file error: pos-ms: {}, duration: {}", m_id, posMs, m_duration);
        return -1;
    }

    // 超过文件总长度，跳到文件结尾
    if (posMs >= (uint64_t)m_duration / 1000)
    {
        posMs = (uint64_t)m_duration / 1000;
    }

    //转换成微秒
    posMs = posMs * 1000;

    av_seek_frame(m_ifmtCtx, -1, posMs, 0);
    m_seekedFile = true;

    SPDINFO("[stream-in][{}] seek file end, pos-ms: {}, duration: {}", m_id, posMs, m_duration);

    return 0;
}

uint64_t StreamInImp::current()
{
    return m_audioCurrent / 1000;
}

uint64_t StreamInImp::total()
{
    return m_duration / 1000;
}

int StreamInImp::sampleRate()
{
    return m_sampleRate;
}

int StreamInImp::bitrate()
{
    return m_bitrate;
}

int StreamInImp::channel()
{
    return m_channel;
}

AudioCodec_t StreamInImp::audioCodec()
{
    return m_audioCodec;
}

int StreamInImp::audioAvCodec()
{
    return m_audioAvCodec;
}

int StreamInImp::statiCallBack(void * p)
{
    StreamInImp * opaque = static_cast<StreamInImp*>(p);
    if (NULL == opaque)
    {
        return 1;
    }
    return opaque->callback();
}

int StreamInImp::callback()
{
    uint64_t now = GetMillisecondCounter();
    if (now > m_interruptStartTickCount
        && now - m_interruptStartTickCount > m_streamInTimeout)
    {
        SPDERROR("[stream-in][{}] get stream timeout, now: {}, m_interruptStartTickCount: {}", 
            m_id, now, m_interruptStartTickCount);
        return 1;
    }
    return 0;
}


int StreamInImp::readAvPacket(AVPacket *pkt)
{
    if (NULL == m_ifmtCtx)
    {
        return -1;
    }

    m_interruptStartTickCount = GetMillisecondCounter();
    int ret = 0;
    ret = av_read_frame(m_ifmtCtx, pkt);
    if (ret < 0)
    {
        // retry
        ret = av_read_frame(m_ifmtCtx, pkt);
    }
    if (ret < 0)
    {
        SPDERROR("[stream-in][{}] av_read_frame error, ret: {}", m_id, ret);
        return -1;
    }
    m_lastFrameTick = GetMillisecondCounter();
    return ret;
}

int StreamInImp::audiotime(AVPacket *pkt)
{
    AVRational time_base = m_ifmtCtx->streams[pkt->stream_index]->time_base;
    AVRational time_base_q = { 1, AV_TIME_BASE };

    // 媒体时长
    int64_t pts_time = av_rescale_q(pkt->dts, time_base, time_base_q);
    if (-1 == m_startPtsTime)
    {
        m_startPtsTime = pts_time;
    }
    pts_time = pts_time - m_startPtsTime;

    // 帧时长
    m_frameDuration = pts_time - m_audioCurrent;

    // 当前时间
    int64_t now_time = av_gettime();

    // 若跳转到指定时间点，则更新起始时间戳
    if (m_seekedFile)
    {
        m_pauseTime = 0;
        m_starttime = now_time - pts_time;
        m_seekedFile = false;
    }

    // 已经过的时长
    resume();
    int64_t sdur = now_time - m_starttime - m_pauseTime;

    if (sdur < pts_time)
    {
        av_usleep((uint32_t)(pts_time - sdur));
    }

    // 记录媒体时长
    m_audioCurrent = pts_time;

    return 0;
}

void StreamInImp::lastFrameDelay()
{
    // 当前时间
    int64_t now_time = av_gettime();
    // 已经过的时长
    resume();
    int64_t sdur = now_time - m_starttime - m_pauseTime;
    int64_t pts_time = m_audioCurrent + m_frameDuration;
    if (sdur < pts_time)
    {
        av_usleep((uint32_t)(pts_time - sdur));
    }
    // 记录媒体时长
    m_audioCurrent = pts_time;
}

void StreamInImp::pause()
{
    m_pauseTick = av_gettime();
}

void StreamInImp::resume()
{
    if (m_pauseTick != 0)
    {
        m_pauseTime += (av_gettime() - m_pauseTick);
        m_pauseTick = 0;
    }
}