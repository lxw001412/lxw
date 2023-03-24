#pragma once

#include <assert.h>
#include <string>
#include <vector>
#include <sstream>

//ffmpeg
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h> 
#include <libswscale/swscale.h> 
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
};

#include "objPool.h"
#include "objlist.h"

/**
  * @brief 编解码模块初始化
  *
  * @return 0：成功，其他值：失败
*/
int CodecInit(void);

/**
  * @brief 编解码模块清理，释放内存
  *
*/
void CodecCleanup(void);

enum CoderID
{
	Coder_None = 0,
	/*
		Audio
	*/
	Coder_PCM = AV_CODEC_ID_PCM_S16LE,
	Coder_MP3 = AV_CODEC_ID_MP3,
    Coder_MP2 = AV_CODEC_ID_MP2,
	Coder_G711 = AV_CODEC_ID_PCM_MULAW,
	Coder_AAC = AV_CODEC_ID_AAC,
	Coder_IMA = AV_CODEC_ID_ADPCM_IMA_WAV,

	/*
		Video
	*/
	Coder_H264 = AV_CODEC_ID_H264,
};


#define DATA_STREAM_EX_BUFF_UNIT_SIZE 512

//数据存储结构体
class DATA_STREAM : public ObjList
{
public:
	virtual ~DATA_STREAM()
	{
		if (NULL != m_buf)
		{
			delete[] m_buf;
			m_buf = NULL;
		}
	}

	const DATA_STREAM& operator = (const DATA_STREAM & obj)
	{
		copy(obj.m_buf, obj.m_size);
		m_pts_time = obj.m_pts_time;
		m_seq = obj.m_seq;
		m_flag = obj.m_flag;
		return *this;
	}

	inline void copy(const void* data, uint32_t size)
	{
		if (m_buffSize < size)
		{
			if (m_buf != NULL)
			{
				delete[]m_buf;
			}
			m_buffSize = ((size - 1) / DATA_STREAM_EX_BUFF_UNIT_SIZE + 1) * DATA_STREAM_EX_BUFF_UNIT_SIZE;
			m_buf = new uint8_t[m_buffSize];
			if (m_buf == NULL)
			{
				exit(1);
			}
		}
		if (size > 0 && data != NULL)
		{
			memcpy(m_buf, data, size);
		    m_size = size;
		}
		else
		{
		    m_size = 0;
		}
	}

	inline void append(const void* data, uint32_t size)
	{
	    assert(data != NULL);
	    if (0 == size)
	    {
	        return;
	    }
		if (m_buffSize < size + m_size)
		{
			m_buffSize = ((size + m_size - 1) / DATA_STREAM_EX_BUFF_UNIT_SIZE + 1) * DATA_STREAM_EX_BUFF_UNIT_SIZE;
			uint8_t *temp = new uint8_t[m_buffSize];
			if (temp == NULL)
			{
				exit(1);
			}
			if (m_size > 0)
			{
			    memcpy(temp, m_buf, m_size);
			}
			if (m_buf != NULL)
			{
				delete[]m_buf;
			}
			m_buf = temp;
		}
	    memcpy(m_buf + m_size, data, size);
		m_size += size;
	}

	inline const uint8_t* data() const
	{
		return m_buf;
	}

	inline uint8_t* _data()
	{
		return m_buf;
	}

	inline uint32_t size() const
	{
		return m_size;
	}

	inline void clear()
	{
		m_size = 0;
	}

	inline int64_t pts_time() const
	{
		return m_pts_time;
	}

	inline void pts_time(uint64_t v)
	{
		m_pts_time = v;
	}

	inline int16_t seq() const
	{
		return m_seq;
	}

	inline void seq(uint16_t v)
	{
		m_seq = v;
	}

	inline int flag() const
	{
		return m_flag;
	}

	inline void flag(int v)
	{
		m_flag = v;
	}


protected:
	DATA_STREAM() : ObjList(), m_buf(NULL), m_buffSize(0), m_size(0), m_pts_time(0), m_seq(0), m_flag(0)
	{
	}

	DATA_STREAM(uint32_t size) : ObjList(), m_size(0), m_pts_time(0), m_seq(0), m_flag(0)
	{
        if (size > 0)
        {
            m_buf = new uint8_t[size];
        }
        else
        {
            m_buf = NULL;
        }
		m_buffSize = size;
	}

	friend class ObjPollManager<DATA_STREAM, uint32_t>;

protected:
	uint8_t *m_buf;
	uint32_t m_buffSize;
	uint32_t m_size;
	int64_t m_pts_time;
	uint16_t m_seq;
	int m_flag;
};

typedef ACE_Singleton<ObjPollManager<DATA_STREAM, uint32_t>, ACE_Mutex> DATA_STREAM_POOL;

#define DataStreamAutoRelease(instance) \
impl_DataStreamAutoRelease _data_stream_auto_release_##instance(&instance)

class impl_DataStreamAutoRelease
{
public:
    impl_DataStreamAutoRelease(DATA_STREAM **p) : m_p(p) {};
    virtual ~impl_DataStreamAutoRelease() {
        if (*m_p != NULL)
        {
            DATA_STREAM_POOL::instance()->release_obj(*m_p);
        }
    }

private:
    impl_DataStreamAutoRelease() {};

    DATA_STREAM **m_p;
};

struct TLV_TYPE
{
	TLV_TYPE():type(0){}

	unsigned char type;
	std::vector<std::string> values;
};

//将uint64转换成std::string
std::string __Uint64toString(const uint64_t obj);

struct Media_Attr
{
	Media_Attr()
	{
		mediaId = Coder_None;
		channals = 0;
		sampleRate = 0;
		bitRate = 64000;
		width = 0;
		high = 0;
		fps = 0;
		av_fmt = AV_SAMPLE_FMT_S16;
	}

	bool operator == (const Media_Attr & obj) const
	{
		std::string tmp;
		tmp = __Uint64toString(mediaId)+"+"+__Uint64toString(channals)+"+"+__Uint64toString(sampleRate)
				+"+" + __Uint64toString(bitRate);
		std::string tmp1;
		tmp1 = __Uint64toString(obj.mediaId)+"+"+__Uint64toString(obj.channals)+"+"+__Uint64toString(obj.sampleRate)
			+"+" + __Uint64toString(obj.bitRate);
		return tmp == tmp1;
	}

	bool operator < (const Media_Attr & obj) const
	{
		std::string tmp;
		tmp = __Uint64toString(mediaId)+"+"+__Uint64toString(channals)+"+"+__Uint64toString(sampleRate)
			+"+" + __Uint64toString(bitRate) + "+" + __Uint64toString(fps) + "+" + __Uint64toString(width) + "+" + __Uint64toString(high);
		std::string tmp1;
		tmp1 = __Uint64toString(obj.mediaId)+"+"+__Uint64toString(obj.channals)+"+"+__Uint64toString(obj.sampleRate)
			+"+" + __Uint64toString(obj.bitRate)+ "+" + __Uint64toString(obj.fps) + "+" + __Uint64toString(obj.width) + "+" 
			+ __Uint64toString(obj.high);;
		return tmp < tmp1;
	}

	CoderID mediaId;
	int channals;
	int sampleRate;
	int bitRate;
	
	int width;
	int high;
	int fps;
	AVSampleFormat av_fmt;
};


//获取合适的样本
AVSampleFormat GetBestSampleFormat(AVCodec * pCodec, AVSampleFormat id);

//获取合适的采样率
int GetBestSampleRate(AVCodec * pCodec, int sample_rate);

uint64_t GetBestChannlLayout(AVCodec * pCodec, uint64_t lay_out);

#ifdef WIN32
//ffmpeg
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "swscale.lib")
//end
#endif
