#pragma once
#include "baseEncoder.h"

class AudioEncoder
	: public BaseEncoder
{
	typedef BaseEncoder Base;
public:
	AudioEncoder(void);
	virtual ~AudioEncoder(void);

	virtual int open(const Media_Attr & eid);

	virtual int close();

	virtual int encode(std::vector<DATA_STREAM *> &OutPut, AVFrame * &frame);

	int modify(const Media_Attr & eid);
protected:
	int __swr_filo_add(AVFrame * &frame);
	int __init_fifo(void);
	int __add_samples_to_fifo(AVFrame * &frame);
	int __add_samples_to_fifo(uint8_t **converted_input_samples, const int frame_size);
	int __init_swr(AVFrame * &frame);
	int __update_coder(AVFrame * &frame);
	int __make_code(std::vector<DATA_STREAM *> &OutPut);

	AAC_ADTS * m_adts;

	AVAudioFifo * m_fifo;

	SwrContext * m_SwrCtx; //转换

	///////////////////////////////////////////
	int m_pre_sample_rate;

	uint64_t m_pre_channel_layout;

	int m_pre_channels;

	int m_pre_format;

};
