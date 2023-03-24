#pragma once
#include "codecDef.h"

class BaseDecoder
{
public:
	BaseDecoder(void);
	virtual ~BaseDecoder(void);

	virtual int open(const Media_Attr & did);

	virtual int close(void);

	/*
		pFrame 必须申请空间
	*/
	virtual int decode(DATA_STREAM * inbuf, AVFrame * &pFrame);
	virtual int decode(uint8_t* inbuf, uint32_t size, uint64_t pts_time, AVFrame * &pFrame);

protected:
	AVCodecContext * m_deCodecCtx;

	uint8_t * m_buf;

	Media_Attr m_DecodeInfo;
};
