#include "baseDecoder.h"


BaseDecoder::BaseDecoder(void)
{
	m_deCodecCtx = NULL;
	m_buf = NULL;
}

BaseDecoder::~BaseDecoder(void)
{
	if(NULL != m_deCodecCtx)
	{
		avcodec_close(m_deCodecCtx);
		avcodec_free_context(&m_deCodecCtx);
		m_deCodecCtx = NULL;
	}
	if(NULL != m_buf)
	{
		delete [] m_buf;
		m_buf = NULL;
	}
}

int BaseDecoder::open(const Media_Attr & did)
{

	return 0;
}

int BaseDecoder::close(void)
{
	return 0;
}

int BaseDecoder::decode(DATA_STREAM * inbuf, AVFrame * &pFrame)
{
	return 0;
}

int BaseDecoder::decode(uint8_t* inbuf, uint32_t size, uint64_t pts_time, AVFrame * &pFrame)
{
	return 0;
}
