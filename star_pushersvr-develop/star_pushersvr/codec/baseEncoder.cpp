#include "baseEncoder.h"

BaseEncoder::BaseEncoder(void)
{
	m_isChange = false;

}

BaseEncoder::~BaseEncoder(void)
{
	if(NULL != m_CodecCtx)
	{
		avcodec_close(m_CodecCtx);
		//avcodec_free_context(&m_CodecCtx);
		av_free(m_CodecCtx);
		m_CodecCtx = NULL;
	}
	
}

int BaseEncoder::open(const Media_Attr & eid)
{
	return 0;
}

int BaseEncoder::close()
{
	return 0;
}

int BaseEncoder::encode(std::vector<DATA_STREAM *> &OutPut, AVFrame * &frame)
{
	return 0;
}

int BaseEncoder::modify(int bitrate)
{
	return 0;
}