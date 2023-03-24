#include "audioEncoder.h"

AudioEncoder::AudioEncoder(void)
{
	m_CodecCtx = NULL;
	m_fifo = NULL;
	m_SwrCtx = NULL;
	m_adts = NULL;
	m_pre_sample_rate = 0;
	m_pre_channel_layout = 0;
	m_pre_channels = 0;
}

AudioEncoder::~AudioEncoder(void)
{
	if(NULL != m_fifo)
	{
		av_audio_fifo_free(m_fifo);
		m_fifo = NULL;
	}
	if(NULL != m_adts)
	{
		delete m_adts;
		m_adts =NULL;
	}
	if(NULL != m_SwrCtx)
	{
		swr_free(&m_SwrCtx);
		m_SwrCtx = NULL;
	}
}

int AudioEncoder::open(const Media_Attr & eid)
{
	if(NULL != m_CodecCtx)
	{
		return -1;
	}

	m_encodeInfo = eid;
	if(eid.mediaId == Coder_None)
	{
		return -1;
	}

    AVCodec *p = NULL;
    if (m_encodeInfo.mediaId == (int)AV_CODEC_ID_AAC)
    {
        p = avcodec_find_encoder((AVCodecID)m_encodeInfo.mediaId);
    }
    else
    {
        p = avcodec_find_encoder((AVCodecID)m_encodeInfo.mediaId);
    }
	if (NULL == p)
	{
		return -1;
	}
	m_CodecCtx = avcodec_alloc_context3(p);
	if(NULL == m_CodecCtx)
	{
		return -1;
	}
	//m_CodecCtx->codec = NULL;
	if(m_encodeInfo.mediaId == (int)AV_CODEC_ID_AAC)
	{
		if (m_adts != NULL)
		{
			delete m_adts;
		}
		m_adts = new AAC_ADTS;
		// av_opt_set(m_CodecCtx, "profile", "aac_low", 0);
        m_CodecCtx->profile = FF_PROFILE_AAC_LOW;
	}
	
	return 0;
}

int AudioEncoder::close()
{
	Base::close();
	return 0;
}

int AudioEncoder::modify(const Media_Attr & eid)
{
	m_isChange = true;
	m_encodeInfo = eid;
	return 0;
}

int AudioEncoder::encode(std::vector<DATA_STREAM *> &OutPut, AVFrame * &frame)
{
	int ret = 0;
	if(frame->nb_samples == 0)
	{
		return ret;
	}
	ret = this->__update_coder(frame);
	if(ret < 0)
	{
		return ret;
	}
	ret = this->__swr_filo_add(frame);
	if(ret < 0)
	{
		return ret;
	}

	while(1)
	{
		ret = this->__make_code(OutPut);
		if(ret < 0)
		{
			return -1;
		}

		ret=av_audio_fifo_size(m_fifo);
		if(ret < m_CodecCtx->frame_size)
		{
			break;
		}
	}

	return 0;
}

int AudioEncoder::__init_fifo(void)
{
	if(NULL != m_fifo)
	{
		av_audio_fifo_free(m_fifo);
		m_fifo = NULL;
	}
	/** Create the FIFO buffer based on the specified output sample format. */
	if (!(m_fifo = av_audio_fifo_alloc(m_CodecCtx->sample_fmt,
		m_CodecCtx->channels, m_CodecCtx->frame_size))) 
	{
		return -1;
	}

	return 0;
}

int AudioEncoder::__add_samples_to_fifo(AVFrame * &frame)
{
	int error = 0;

	/**
     * Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples.
     */
    if ((error = av_audio_fifo_realloc(m_fifo, av_audio_fifo_size(m_fifo) + frame->nb_samples)) < 0)
	{
        return error;
    }

	int ret = 0;
    /** Store the new samples in the FIFO buffer. */
    if ((ret = av_audio_fifo_write(m_fifo, (void **)frame->extended_data,
                            frame->nb_samples) )< frame->nb_samples)
	{
        return -1;
    }
	ret = av_audio_fifo_size(m_fifo);

    return ret;
}

int AudioEncoder::__add_samples_to_fifo(uint8_t **converted_input_samples, const int frame_size)
{
	int error;

    /**
     * Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples.
     */
    if ((error = av_audio_fifo_realloc(m_fifo, av_audio_fifo_size(m_fifo) + frame_size)) < 0)
	{
        return error;
    }

    /** Store the new samples in the FIFO buffer. */
	int ret = 0;
    if ((ret=av_audio_fifo_write(m_fifo, (void **)converted_input_samples,
                            frame_size)) < frame_size)
	{
        return -1;
    }
    return ret;
}

int AudioEncoder::__init_swr(AVFrame * &frame)
{
	if(NULL != m_SwrCtx)
	{
		swr_free(&m_SwrCtx);
		m_SwrCtx = NULL;
	}

	if(m_CodecCtx->channel_layout != frame->channel_layout|| m_CodecCtx->sample_rate != frame->sample_rate 
		|| m_CodecCtx->sample_fmt  != frame->format)
	{
		m_SwrCtx = swr_alloc();
		m_SwrCtx = swr_alloc_set_opts(m_SwrCtx, m_CodecCtx->channel_layout, m_CodecCtx->sample_fmt, m_CodecCtx->sample_rate, 
			frame->channel_layout, (AVSampleFormat)frame->format, frame->sample_rate, 0, NULL);

		swr_init(m_SwrCtx);	

		return 0;
	}

	return -1;
}

int AudioEncoder::__update_coder(AVFrame * &frame)
{
	int ret = 0;
	if(frame->sample_rate != m_pre_sample_rate || frame->channels!=m_pre_channels
		|| frame->channel_layout!=m_pre_channel_layout || frame->format!=m_pre_format 
		|| m_isChange)
	{
		m_isChange = false;
		m_pre_sample_rate = frame->sample_rate;
		m_pre_channels = frame->channels;
		m_pre_channel_layout = frame->channel_layout;
		m_pre_format = frame->format;

		AVCodec * p = avcodec_find_encoder((AVCodecID)m_encodeInfo.mediaId);
		if (NULL == p)
		{
			return -1;
		}
		m_CodecCtx->sample_fmt = GetBestSampleFormat(p, (AVSampleFormat)AV_SAMPLE_FMT_S16P);
		m_CodecCtx->bit_rate = m_encodeInfo.bitRate;
		m_CodecCtx->sample_rate = GetBestSampleRate(p, m_encodeInfo.sampleRate);
		if(1 == m_encodeInfo.channals)
		{
			m_CodecCtx->channel_layout = AV_CH_LAYOUT_MONO;
			m_CodecCtx->channels = av_get_channel_layout_nb_channels(m_CodecCtx->channel_layout);
		}
		else if(2 == m_encodeInfo.channals)
		{
			m_CodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
			m_CodecCtx->channels = av_get_channel_layout_nb_channels(m_CodecCtx->channel_layout);
		}
	
		AVRational time_base_q={1,m_CodecCtx->sample_rate};
		m_CodecCtx->time_base = time_base_q;

        if (m_encodeInfo.mediaId == (int)AV_CODEC_ID_AAC)
        {
            m_CodecCtx->profile = FF_PROFILE_AAC_LOW;
        }

		if(NULL != m_CodecCtx->codec)
		{
			avcodec_close(m_CodecCtx);
		}

		//p->capabilities = AV_CODEC_CAP_VARIABLE_FRAME_SIZE | AV_CODEC_CAP_LOSSLESS;
		ret = avcodec_open2(m_CodecCtx, p, NULL);
		if(ret < 0)
		{
			return ret;
		}

		if(NULL != m_adts)
		{
			m_adts->sample_rate_index(m_encodeInfo.sampleRate);

			m_adts->channel_configuration(m_encodeInfo.channals);
		}

		this->__init_fifo();
		this->__init_swr(frame);
	}

	return ret;
}

int AudioEncoder::__swr_filo_add(AVFrame * &frame)
{
	int ret = 0;
	if(NULL != m_SwrCtx)
	{
		AVFrame * pf  = NULL;
		pf = av_frame_alloc();
		pf->channel_layout = m_CodecCtx->channel_layout;
		pf->sample_rate = m_CodecCtx->sample_rate;
		pf->format = m_CodecCtx->sample_fmt;
		swr_convert_frame(m_SwrCtx, pf, frame);
		ret = this->__add_samples_to_fifo(pf);
		av_frame_free(&pf);
	}
	else
	{
		ret = this->__add_samples_to_fifo(frame);
	}

	return ret;
}

int AudioEncoder::__make_code(std::vector<DATA_STREAM *> &OutPut)
{
	int ret = 0;
	AVPacket *enc_pkt;
    enc_pkt = av_packet_alloc();
    if (NULL == enc_pkt)
    {
        return -1;
    }
	enc_pkt->data = NULL;
	enc_pkt->size = 0;

	AVFrame *output_frame = NULL;
	output_frame = av_frame_alloc();
	output_frame->nb_samples = m_CodecCtx->frame_size;
	output_frame->format = m_CodecCtx->sample_fmt;
	output_frame->channel_layout = m_CodecCtx->channel_layout;
	output_frame->sample_rate = m_CodecCtx->sample_rate;
	av_frame_get_buffer(output_frame, 0);

	ret = av_audio_fifo_size(m_fifo);
	if(ret >= output_frame->nb_samples)
	{
		if ((ret = av_audio_fifo_read(m_fifo, (void **)output_frame->data, output_frame->nb_samples)) < 0)
		{
			av_frame_free(&output_frame);
			return -1;
		}
	}
	else
	{
		av_frame_free(&output_frame);
		return 0;
	}

    ret = avcodec_send_frame(m_CodecCtx, output_frame);
    if (0 == ret)
    {
        ret = avcodec_receive_packet(m_CodecCtx, enc_pkt);
    }

    if (0 == ret)
	{
		DATA_STREAM * p = DATA_STREAM_POOL::instance()->get_obj();
		
		int ipos = 0;
		if(NULL != m_adts && !(enc_pkt->data[0]==0xff && ((enc_pkt->data[1]>>4)==0x0F)))
		{
			ipos = 7;
			m_adts->aac_frame_length(enc_pkt->size + ipos);
			p->copy(m_adts->Data(), ipos);
		}

		p->append(enc_pkt->data, enc_pkt->size);
		OutPut.push_back(p);
	}

    av_frame_free(&output_frame);
    av_packet_free(&enc_pkt);
	return 0;
}