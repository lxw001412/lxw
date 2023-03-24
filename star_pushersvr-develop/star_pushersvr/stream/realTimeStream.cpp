#include "realTimeStream.h"
#include "streamInImp.h"
#include "streamOutRtmp.h"
#include "spdlogging.h"
#include "tickcount.h"
#include "AppConfig.h"
#include "Module_Interface.h"
#include "audioParser.h"
#include "autofree.h"
#ifdef ENABLE_RTSP
#include "streamOutRtsp.h"
#endif // ENABLE_RTSP

#include <assert.h>
#include <sstream>
#include <json/json.h>

#define REPORT_PROCESS_INTERVAL 1

// 音频解码缓存大小
#define AUDIO_DECODE_INIT_BUFFER_SIZE 10240

RealTimeStream::RealTimeStream() : StreamBase(),
    m_streamIn(nullptr),
    m_streamOut(nullptr), 
    m_codec(AC_AAC),
    m_channel(0),
    m_sampleRate(0),
    m_streamOutErrorTimes(0),
    m_lastReportProcessTick(0),
    m_pauseTime(0),
    m_stop(false),
    m_enableReportStop(true)
{
    m_outputBufferSize = AUDIO_DECODE_INIT_BUFFER_SIZE;
    m_outputBuffer = new uint8_t[m_outputBufferSize];

    m_streamReconnectTimes = DEFAULT_STREAM_RECONNECT_TIMES;
    m_streamReconnectIntervalMilliseconds = DEFAULT_STREAM_RECONNECT_INTERVAL_MILLISECONDS;
    AppConfig::instance()->getIntParam("stream", "ReconnectTimes", m_streamReconnectTimes);
    AppConfig::instance()->getIntParam("stream", "ReconnectIntervalMilliseconds", m_streamReconnectIntervalMilliseconds);
}

RealTimeStream::~RealTimeStream()
{
    SPDINFO("[rtstream][{}] destruct", m_info.id());
    if (NULL != m_streamIn)
    {
        delete m_streamIn;
    }
    if (NULL != m_streamOut)
    {
        delete m_streamOut;
    }
    if (NULL != m_outputBuffer)
    {
        delete[]m_outputBuffer;
        m_outputBuffer = NULL;
    }
}


int RealTimeStream::start(const StreamInfo *streamInfo)
{
    assert(streamInfo != NULL);
    std::string info;
    SPDINFO("[rtstream][{}] stream start, info: {}", streamInfo->id(), streamInfo->print(info));

	m_stop = false;
	const StreamInfoImp* impl = dynamic_cast<const StreamInfoImp*>(streamInfo);
	if (impl == nullptr)
	{
        SPDERROR("[rtstream][{}] dynamic cast error", streamInfo->id());
		return 1;
	}

	m_info = *impl;
	m_thread = std::thread(&RealTimeStream::svr, this);
	return 0;
}


int RealTimeStream::stop(bool reportStop)
{
	m_enableReportStop = reportStop;
	if (m_stop)
	{
		return 0;
	}

    SPDINFO("[rtstream][{}] stream stopping...", m_info.id());
	m_stop = true;
	m_thread.join();
    SPDINFO("[rtstream][{}] stream stopped", m_info.id());
	return 0;
}

const StreamInfo* RealTimeStream::info()
{
	return &m_info;
}

int RealTimeStream::pause()
{
    SPDWARN("[rtstream][{}] unsupport pause", m_info.id());
	return 0;
}

int RealTimeStream::resume()
{
    SPDWARN("[rtstream][{}] unsupport resume", m_info.id());
	return 0;
}

int RealTimeStream::jump(int index, int pos,StreamIn* streamin)
{
    SPDWARN("[rtstream][{}] unsupport jump", m_info.id());
	return 0;
}

int RealTimeStream::initStreamOut()
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
        SPDERROR("[rtstream][{}] init stream out failed: ret: {}, url: {}", m_info.id(), rc, m_info.url());
    }

	return rc;
}

bool RealTimeStream::initStream()
{
	bool bRet = false;
	do
	{
		m_streamIn = new StreamInImp();
        int rc = m_streamIn->open(m_info.id(), m_info.source());
		if (0 != rc)
		{
			std::stringstream ss;
			ss << "realtime stream in open failed, source=" << m_info.source() << ", rc: " << rc;
			setLastError(SEC_STREAMIN, ss.str().c_str());
			break;
		}

		m_codec = (AudioCodec_t)(m_streamIn->audioCodec());
		m_channel = m_streamIn->channel();
		m_sampleRate = m_streamIn->sampleRate();

        rc = initStreamOut();
		if (0 != rc)
		{
            std::stringstream ss;
            ss << "realtime stream out init failed, url=" << m_info.url() << ", rc: " << rc;
			setLastError(SEC_STREAMOUT, ss.str().c_str());
			break;
		}

		bRet = true;
	} while (0);
	
	if (!bRet)
	{
		finiStream();
	}

	return bRet;
}


void RealTimeStream::finiStream()
{
	if (m_streamIn != nullptr)
	{
		m_streamIn->close();
		delete m_streamIn;
		m_streamIn = nullptr;
	}

	if (m_streamOut != nullptr)
	{
		m_streamOut->close();
		delete m_streamOut;
		m_streamOut = nullptr;
	}
}

void RealTimeStream::setLastError(int code, const char* msg)
{
	m_info.lastErrorCode(code);
	m_info.lastErrorMsg(msg);
	if (code != 0)
	{
		SPDERROR("[rtstream][{}] stream error: code: {}, msg: {}", m_info.id(), code, msg);
	}
}

int RealTimeStream::pullAndpushFrame()
{
	if (nullptr == m_streamOut 
		|| nullptr == m_streamIn)
	{
		std::stringstream ss;
		ss << "Push data failed, url: " << m_info.url() << ", source: " << m_info.source();
		setLastError(SEC_STREAMOUT, ss.str().c_str());
		return -2;
	}

	FrameData_t fd;
	int rc = m_streamIn->readData(fd);
	if (rc != 0)
	{
        SPDERROR("[realtime stream][{}] stream in read data error, rc: {}, url: {}", m_info.id(), rc, m_info.source());

        // reconnect stream in
        rc = reconnectStreamIn(fd);

        if (rc != 0)
        {
            std::stringstream ss;
            ss << "realtime stream in read data failed, source=" << m_info.source();
            setLastError(SEC_STREAMIN, ss.str().c_str());
            return -1;
        }
	}

    bool audioConvert = false;
    if (m_info.audioConvertParam().enable
        && (m_streamIn->audioCodec() != m_info.audioConvertParam().codec
            || m_streamIn->sampleRate() != m_info.audioConvertParam().sampleRate
            || m_streamIn->bitrate() != m_info.audioConvertParam().bitrate
            || m_streamIn->channel() != m_info.audioConvertParam().channelNum))
    {
        audioConvert = true;
    }

    std::vector<DATA_STREAM *> output;
    if (audioConvert)
    {
        AVFrame * frame = av_frame_alloc();
        int ret = m_audioDecoder.decode(fd.data, fd.length, 0, frame);
        if (0 != ret)
        {
            SPDERROR("[realtime stream][{}] decode frame error: {}", m_info.id(), ret);
            return 0;
        }

        ret = m_audioEncoder.encode(output, frame);
        av_frame_free(&frame);
        if (0 != ret)
        {
            SPDERROR("[realtime stream][{}] encode frame error: {}", m_info.id(), ret);
            return 0;
        }
        if (output.size() == 0)
        {
            return 0;
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

	// 推流持续时长, 就是流当前的时间
	m_info.playSeconds((int)(m_streamIn->current()/1000));

	// 上报进度
	reportProcess();
	return 0;
}


int RealTimeStream::pushFrame(FrameData_t &fd)
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
        SPDERROR("[rtstream][{}] stream out error, url: {}, error times: {}", m_info.id(), m_info.url(), m_streamOutErrorTimes);

        // 推流重连
        for (int i = 0; !m_stop && i < m_streamReconnectTimes; i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(m_streamReconnectIntervalMilliseconds));
            SPDERROR("[rtstream][{}]  reconnect stream out, times: {} url: {}", m_info.id(), i + 1, m_info.url());
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

            SPDINFO("[rtstream][{}] reconnect url ok: {}", m_info.id(), m_info.url());
            break;
        }
        if (0 != rc)
        {
            std::stringstream ss;
            ss << "stream out reconnect failed, url: " << m_info.url() << ", source: " << m_info.source();
            setLastError(SEC_STREAMOUT, ss.str().c_str());
            return -2;
        }

        // 重连输入流，会导致部分音频内容丢失，但可以减少延时
        FrameData_t fdTemp;
        reconnectStreamIn(fdTemp);
    }
    return rc;
}

bool RealTimeStream::isPlayOver()
{
	return (m_info.duration() != 0) &&  ((GetUtcCount() - m_info.startTime() - m_pauseTime) >= (unsigned int)m_info.duration());
}

int RealTimeStream::reportStop()
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
	SPDINFO("[rtstream][{}] stop callback: {}", m_info.id(), smsg);

	// 调用回调模块接口
    HttpRequest *request = HttpRequest::create(m_info.stopCallbackUrl(), "POST");
    request->setParam(smsg, "application/json");
    Module_Interface::sendStreamStop(request);

	return 0;
}

int RealTimeStream::reportProcess()
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

	msg["id"] = m_info.id();
	msg["type"] = "realtime";
	msg["playedSeconds"] = m_info.playSeconds();

	std::string smsg = msg.toStyledString();
	SPDDEBUG("[rtstream][{}]  process callback: {}", m_info.id(), smsg);
	
    // 调用回调模块接口
    HttpRequest *request = HttpRequest::create(m_info.processCallbackUrl(), "POST");
    request->setParam(smsg, "application/json");
    Module_Interface::sendStreamProgress(request);

	m_lastReportProcessTick = GetUtcCount();
	return 0;
}

int RealTimeStream::svr()
{
	setLastError(SEC_OK, "");
	m_info.status(SS_RUNNING);

	if (!initStream())
	{
		m_info.status(SS_STOPPED);

		// 上报推流停止
		reportStop();
		return 0;
	}

    if (m_info.audioConvertParam().enable)
    {
        // 初始化编解码器
        int rc = initDecoder();
        if (rc != 0)
        {
            setLastError(SEC_CODEC, "Init decoder failed");
            m_info.status(SS_STOPPED);

            // 上报推流停止
            reportStop();
            return 0;
        }
        rc = initEncoder();
        if (rc != 0)
        {
            setLastError(SEC_CODEC, "Init decoder failed");
            m_info.status(SS_STOPPED);

            // 上报推流停止
            reportStop();
            return 0;
        }
    }

	m_info.startTime(GetUtcCount());
	m_info.playSeconds(0);
	
	// 初始上报
	reportProcess();

	while (!m_stop & !isPlayOver())
	{
		if (0 != pullAndpushFrame())
		{
			break;
		}
	}

    m_info.stopTime(GetUtcCount());
	m_info.status(SS_STOPPED);

	// 上报推流停止
	reportStop();

    finiStream();

	return 0;
}

int RealTimeStream::initDecoder()
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

int RealTimeStream::initEncoder()
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

int RealTimeStream::reallocOutputBuff(int size)
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

int RealTimeStream::reconnectStreamIn(FrameData_t &fd)
{
    int rc = 0;
    for (int i = 0; i < m_streamReconnectTimes; i++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(m_streamReconnectIntervalMilliseconds));

        SPDERROR("[realtime stream][{}] reconnect stream in, retry: {}, url: {}", m_info.id(), i + 1, m_info.source());

        if (NULL != m_streamIn)
        {
            delete m_streamIn;
        }
        m_streamIn = new StreamInImp();
        if (NULL == m_streamIn)
        {
            std::stringstream ss;
            ss << "realtime stream in read data failed, source=" << m_info.source();
            setLastError(SEC_STREAMIN, ss.str().c_str());
            return -1;
        }
        rc = m_streamIn->open(m_info.id(), m_info.source());
        if (rc != 0)
        {
            SPDERROR("[realtime stream][{}] reconnect stream in, open stream failed, rc: {}, url: {}", m_info.id(), rc, m_info.source());
            continue;
        }
        rc = m_streamIn->readData(fd);
        if (rc != 0)
        {
            SPDERROR("[realtime stream][{}] reconnect stream in, read data failed, rc: {}, url: {}", m_info.id(), rc, m_info.source());
            continue;
        }
    }
    return rc;
}