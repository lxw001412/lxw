#pragma once
#include "baseDecoder.h"

class AudioDecoder
	:public BaseDecoder
{
	typedef BaseDecoder Base;
public:
	AudioDecoder(void);
	virtual ~AudioDecoder(void);

	virtual int open(const Media_Attr & did);

	virtual int close(void);

	/*
		pFrame 必须申请空间
	*/
	virtual int decode(DATA_STREAM * inbuf, AVFrame * &pFrame);
	virtual int decode(uint8_t* inbuf, uint32_t size, uint64_t pts_time, AVFrame * &pFrame);


protected:
	void init_filter(void);

protected:
	AVFilterContext *m_buffersink_ctx;
	AVFilterContext *m_buffersrc_ctx;
	AVFilterGraph *m_filter_graph;
    AVPacket *m_decodePacket;
};
