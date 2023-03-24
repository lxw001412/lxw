/**
 * @file streamOutRtsp.cpp
 * @brief RTSP流输出类
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-11-29
 * @description RTSP流输出类
 */

#ifdef ENABLE_RTSP

#include "streamOutRtsp.h"
#include "spdlogging.h"
#include "tickcount.h"
extern "C"
{
#include <libavutil/time.h>
}

StreamOutRtsp::StreamOutRtsp() : 
    m_handler(NULL), 
    m_timestamp(0), 
    m_lastFrameTick(0), 
    m_startTick(0),
    m_codec(AC_UNSUPPORT), 
    m_rtspPusherState(PUSHER_STATE_CONNECTING),
    m_debug(0)
{

}

StreamOutRtsp::~StreamOutRtsp()
{
    SPDINFO("[stream-out-rtsp][{}] destruct", m_id);
    close();
}

int StreamOutRtsp::open(const std::string &id, const std::string &file, AudioCodec_t codec, int channel, int sampleRate, int sampleBits)
{
    m_id = id;
    m_codec = codec;
    m_rtspPusherState = PUSHER_STATE_CONNECTING;

    SPDINFO("[stream-out-rtsp][{}] opening {}, fmt: c:{}, ch:{}, sr:{}, sb:{}", m_id, file, codec, channel, sampleRate, sampleBits);

    close();

    m_url = file;
    m_mi.audioChannel = channel;
    m_mi.audioSamplerate = sampleRate;
    switch (codec)
    {
    case AC_AAC:
        m_mi.audioCodec = RTSP_AUDIO_CODEC_AAC;
        break;
    case AC_MP3:
        m_mi.audioCodec = RTSP_AUDIO_CODEC_MP3;
        break;
    default:
        SPDERROR("[stream-out-rtsp][{}] unsupport media codec: {}", m_id, codec);
        return -1;
        break;
    }

    m_handler = RTSP_Pusher_Create();
    if (NULL == m_handler)
    {
        SPDERROR("[stream-out-rtsp][{}] create rtsp pusher failed, url: {}", m_id, m_url);
        return -2;
    }
    int rc = RTSP_Pusher_SetCallback(m_handler, StreamOutRtsp::callback, this);
    if (0 != rc)
    {
        SPDERROR("[stream-out-rtsp][{}] set rtsp callback failed, url: {}, ret:{}", m_id, m_url, rc);
        return -3;
    }
    rc = RTSP_Pusher_StartStream(m_handler, m_url.c_str(), RTP_OVER_TCP, NULL, NULL, 1, &m_mi);
    if (0 != rc)
    {
        SPDERROR("[stream-out-rtsp][{}] start rtsp stream failed, url: {}, ret:{}", m_id, m_url, rc);
        return -4;
    }

    SPDINFO("[stream-out-rtsp][{}] opened {}, fmt: c:{}, ch:{}, sr:{}, sb:{}", m_id, file, codec, channel, sampleRate, sampleBits);

    return 0;
}

int StreamOutRtsp::close()
{
    if (m_handler != NULL)
    {
        SPDINFO("[stream-out-rtsp][{}] close {}", m_id, m_url);
        RTSP_Pusher_CloseStream(m_handler);
        RTSP_Pusher_Release(m_handler);
        m_handler = NULL;
    }
    return 0;
}

int StreamOutRtsp::writeData(FrameData_t &frameData)
{
    if (PUSHER_STATE_CONNECTING == m_rtspPusherState)
    {
        return 0;
    }

    int rc = 0;
    if (m_startTick == 0)
    {
        m_startTick = GetMillisecondCounter();
    }
    RTSP_MediaFrame frame;
    frame.frameData = frameData.data;
    frame.frameLen = frameData.length;
    frame.timestampSec = (uint32_t)(m_timestamp / 1000000);
    frame.timestampUsec = (uint32_t)(m_timestamp % 1000000);

    m_timestamp += frameData.duration;

    if (m_debug == 0)
    {
        m_debug++;
        SPDINFO("++++++++++ {} {} {} {}", frameData.data[0], frameData.data[1], frameData.data[2], frameData.data[3]);
    }

    rc = RTSP_Pusher_PushFrame(m_handler, &frame);
    if (0 != rc)
    {
        SPDERROR("[stream-out-rtsp][{}] rtsp stream push data failed, rc: {}, url: {}", m_id, rc, m_url);
        SPDINFO("=========={} {} {} {}", frameData.length, frameData.duration, frame.timestampSec, frame.timestampUsec);
        return -4;
    }
    SPDTRACE("[stream-out-rtsp][{}] pts: {}, time: {}, delta: {}", 
        m_id, 
        m_timestamp, 
        (int64_t)(GetMillisecondCounter() - m_startTick), 
        m_timestamp/1000 - (int64_t)(GetMillisecondCounter() - m_startTick));

    return rc;
}

int StreamOutRtsp::handleCallback(RTSP_Pusher_State type, int rtspStatusCode)
{
    if (m_rtspPusherState != type)
    {
        SPDINFO("[stream-out-rtsp][{}] rtsp state callback, state:{}, code:{}", m_id, type, rtspStatusCode);
        m_rtspPusherState = type;
    }
    return 0;
}

int StreamOutRtsp::callback(RTSP_Pusher_State type, int rtspStatusCode, void *obj)
{
    StreamOutRtsp *streamOut = static_cast<StreamOutRtsp*>(obj);
    if (NULL == streamOut)
    {
        return 1;
    }

    return streamOut->handleCallback(type, rtspStatusCode);
}

#endif // ENABLE_RTSP