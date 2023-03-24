#include "codecDef.h"


int CodecInit(void)
{
	avformat_network_init();
    DATA_STREAM_POOL::instance()->init(10240, 0);
    return 0;
}

void CodecCleanup(void)
{
    DATA_STREAM_POOL::instance()->clear();
}


//获取合适的样本
AVSampleFormat GetBestSampleFormat(AVCodec * pCodec, AVSampleFormat id)
{
	int i = 0;
    if (NULL == pCodec->sample_fmts)
    {
        return id;
    }
	while (-1 != pCodec->sample_fmts[i])
	{
		if (id == pCodec->sample_fmts[i])
		{
			return id;
		}
		i++;
	}

	return pCodec->sample_fmts[0];
}

//获取合适的采样率
int GetBestSampleRate(AVCodec * pCodec, int sample_rate)
{
	int i = 0;
    if (NULL == pCodec->supported_samplerates)
    {
        return sample_rate;
    }
	while (0 != pCodec->supported_samplerates[i])
	{
		if (sample_rate == pCodec->supported_samplerates[i])
		{
			return sample_rate;
		}
		i++;
	}
	return pCodec->supported_samplerates[0];
}

uint64_t GetBestChannlLayout(AVCodec * pCodec, uint64_t lay_out)
{
	int i = 0;
    if (NULL == pCodec->channel_layouts)
    {
        return lay_out;
    }
	while (0 != pCodec->channel_layouts[i])
	{
		if (lay_out == pCodec->channel_layouts[i])
		{
			return lay_out;
		}
		++i;
	}

	return pCodec->channel_layouts[0];
}

std::string __Uint64toString(const uint64_t obj)
{
	std::stringstream str;
	str << obj;
	std::string tmp;
	str >> tmp;
	return tmp;
}