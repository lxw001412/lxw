/**
 * @file streamOutRtmp.cpp
 * @brief RTMP流输出类
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-11-29
 * @description RTMP流输出类
 */

#include "streamOutRtmp.h"
#include "spdlogging.h"
#include "tickcount.h"
extern "C"
{
#include <libavutil/time.h>
}

StreamOutRtmp::StreamOutRtmp() : m_handler(NULL), m_timestamp(0), m_lastFrameTick(0), m_startTick(0), m_codec(AC_UNSUPPORT)
{

}

StreamOutRtmp::~StreamOutRtmp()
{
    SPDINFO("[stream-out][{}] destruct", m_id);
    close();
}

int StreamOutRtmp::open(const std::string &id, const std::string &file, AudioCodec_t codec, int channel, int sampleRate, int sampleBits)
{
    m_id = id;
    m_codec = codec;

    SPDINFO("[stream-out][{}] opening {}, fmt: c:{}, ch:{}, sr:{}, sb:{}", m_id, file, codec, channel, sampleRate, sampleBits);

    close();

    m_url = file;
    m_mi.audioChannel = channel;
    m_mi.audioSamplerate = sampleRate;
    m_mi.audioBits = sampleBits;
    switch (codec)
    {
    case AC_AAC:
        m_mi.audioCodec = RTMP_AUDIO_CODEC_AAC;
        break;
    case AC_MP3:
        m_mi.audioCodec = RTMP_AUDIO_CODEC_MP3;
        break;
    case AC_PCM:
        m_mi.audioCodec = RTMP_AUDIO_CODEC_PCM;
        break;
    default:
        SPDERROR("[stream-out][{}] unsupport media codec: {}", m_id, codec);
        return -1;
        break;
    }

    m_handler = RTMP_Pusher_Create();
    if (NULL == m_handler)
    {
        SPDERROR("[stream-out][{}] create rtmp pusher failed, url: {}", m_id, m_url);
        return -2;
    }
    int rc = RTMP_Pusher_SetCallback(m_handler, StreamOutRtmp::callback, this);
    if (RTMP_PUSH_MC_NoErr != rc)
    {
        SPDERROR("[stream-out][{}] set rtmp callback failed, url: {}, ret:{}", m_id, m_url, rc);
        return -3;
    }
    rc = RTMP_Pusher_StartStream(m_handler, m_url.c_str(), &m_mi, RPT_Live);
    if (RTMP_PUSH_MC_NoErr != rc)
    {
        SPDERROR("[stream-out][{}] start rtmp stream failed, url: {}, ret:{}", m_id, m_url, rc);
        return -4;
    }

    SPDINFO("[stream-out][{}] opened {}, fmt: c:{}, ch:{}, sr:{}, sb:{}", m_id, file, codec, channel, sampleRate, sampleBits);

    return 0;
}

int StreamOutRtmp::close()
{
    if (m_handler != NULL)
    {
        SPDINFO("[stream-out][{}] close {}", m_id, m_url);
        RTMP_Pusher_CloseStream(m_handler);
        RTMP_Pusher_Release(m_handler);
        m_handler = NULL;
    }
    return 0;
}

int StreamOutRtmp::writeData(FrameData_t &frameData)
{
    int rc = 0;
    if (m_startTick == 0)
    {
        m_startTick = GetMillisecondCounter();
    }
    if (RTMP_AUDIO_CODEC_PCM == m_mi.audioCodec)
    {
        RTMP_MediaFrame frame;
        int unitSize = 256;
        int offset = 0;
        int packageTime = 5;  // 每次最多推送5ms PCM数据
        unitSize = m_mi.audioSamplerate * (m_mi.audioBits / 8) * m_mi.audioChannel * packageTime / 1000;
        int sampleBytes = (m_mi.audioBits / 8) * m_mi.audioChannel;
        unitSize = (unitSize / sampleBytes) * sampleBytes;
        
        uint32_t timestamp = (uint32_t)(m_timestamp / 1000);
        while (offset < frameData.length)
        {
            int left = frameData.length - offset;
            int length = (left >= unitSize) ? unitSize : left;

            frame.frameData = frameData.data + offset;
            frame.frameLen = length;
            frame.timestamp = timestamp;
            SPDTRACE("[stream-out][{}] Write offset:{}, length:{}, timestamp:{}", m_id, offset, length, timestamp);
            rc = RTMP_Pusher_PushFrame(m_handler, &frame);
            if (RTMP_PUSH_MC_NoErr != rc)
            {
                SPDERROR("[stream-out][{}] rtmp stream push data failed, url: {}", m_id, m_url);
                return -4;
            }
            timestamp += packageTime;
            offset += length;
            if (offset != frameData.length)
            {
                av_usleep((uint32_t)4000);
            }
        }
        m_timestamp += frameData.duration;
    }
    else
    {
        RTMP_MediaFrame frame;
        frame.frameData = frameData.data;
        frame.frameLen = frameData.length;
        frame.timestamp = (uint32_t)(m_timestamp / 1000);
        m_timestamp += frameData.duration;
        rc = RTMP_Pusher_PushFrame(m_handler, &frame);
        if (RTMP_PUSH_MC_NoErr != rc)
        {
            SPDERROR("[stream-out][{}] rtmp stream push data failed, url: {}", m_id, m_url);
            return -4;
        }
        int64_t usedTime = (int64_t)(GetMillisecondCounter() - m_startTick);
        SPDTRACE("[stream-out][{}] pts: {}, time: {}, delta: {}", m_id, m_timestamp, usedTime, m_timestamp/1000 - usedTime);
    }
    return rc;
}

int StreamOutRtmp::handleCallback(RTMP_Pusher_CBType type, void *cbdata)
{
    return 0;
}

int StreamOutRtmp::callback(RTMP_Pusher_CBType type, void *cbdata, void *obj)
{
    StreamOutRtmp *streamOut = static_cast<StreamOutRtmp*>(obj);
    if (NULL == streamOut)
    {
        return 1;
    }

    return streamOut->handleCallback(type, cbdata);
}
