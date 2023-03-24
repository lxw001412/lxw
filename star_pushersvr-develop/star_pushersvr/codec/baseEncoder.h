#pragma once
#include "codecDef.h"
#include "AAC_ADTS.h"

class BaseEncoder
{
public:
	BaseEncoder(void);
	virtual ~BaseEncoder(void);

	virtual int open(const Media_Attr & eid);

	virtual int close();

	virtual int encode(std::vector<DATA_STREAM *> &OutPut, AVFrame * &frame);

	int modify(int bitrate);

protected:
	bool m_isChange;

	AVCodecContext* m_CodecCtx;

	Media_Attr m_encodeInfo;
};
