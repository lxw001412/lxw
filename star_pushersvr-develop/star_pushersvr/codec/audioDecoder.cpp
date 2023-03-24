#include "audioDecoder.h"
#include <iostream>


#define AUDIO_DECODER_BUFF_SIZE 20480

AudioDecoder::AudioDecoder(void)
{
	m_buf = new uint8_t[AUDIO_DECODER_BUFF_SIZE];

	m_buffersink_ctx = NULL;
	m_buffersrc_ctx = NULL;
	m_filter_graph = NULL;
    m_decodePacket = NULL;
}

AudioDecoder::~AudioDecoder(void)
{
	if(NULL != m_filter_graph)
	{
		avfilter_graph_free(&m_filter_graph);
		m_filter_graph = NULL;
	}
    if (NULL != m_decodePacket)
    {
        av_packet_free(&m_decodePacket);
        m_decodePacket = NULL;
    }
}

int AudioDecoder::open(const Media_Attr & did)
{
    if (NULL == m_decodePacket)
    {
        m_decodePacket = av_packet_alloc();
    }
    if (NULL == m_decodePacket)
    {
        return -1;
    }
	if(NULL != m_deCodecCtx)
	{
		if(m_DecodeInfo == did)
		{
			return 0;
		}
		avcodec_close(m_deCodecCtx);
	}

	m_DecodeInfo = did;

	AVCodec * pcode = avcodec_find_decoder((AVCodecID)m_DecodeInfo.mediaId);
	if(NULL == pcode)
	{
		return -1;
	}

	m_deCodecCtx = avcodec_alloc_context3(pcode);
	if(NULL == m_deCodecCtx)
	{
		return -1;
	}
	
	AVRational time_base_q={1, AV_TIME_BASE};  
	m_deCodecCtx->time_base = time_base_q;
	
	if(1 == m_DecodeInfo.channals)
	{
		m_deCodecCtx->channel_layout = AV_CH_LAYOUT_MONO;
		m_deCodecCtx->channels = 1;
	}
	else if(2 == m_DecodeInfo.channals)
	{
		m_deCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
		m_deCodecCtx->channels = 2;
	}

	m_deCodecCtx->sample_rate = m_DecodeInfo.sampleRate;
	if(AV_CODEC_ID_PCM_MULAW == (int)m_DecodeInfo.mediaId || AV_CODEC_ID_PCM_S16LE == (int)m_DecodeInfo.mediaId)
	{
		m_deCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
	}
	else if(AV_CODEC_ID_ADPCM_IMA_WAV == (int)m_DecodeInfo.mediaId)
	{
		m_deCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16P;

		//采样点16位压缩至4位，即4bit表示1个采样点
		m_deCodecCtx->bits_per_coded_sample = 4;
	}

	int ret = avcodec_open2(m_deCodecCtx, pcode, NULL);
	if(ret < 0)
	{
		avcodec_free_context(&m_deCodecCtx);
		m_deCodecCtx = NULL;
		return -1;
	}

	if(AV_CODEC_ID_PCM_MULAW==(AVCodecID)m_DecodeInfo.mediaId)
	{
		this->init_filter();
	}

	return 0;
}

int AudioDecoder::close(void)
{
	Base::close();
	return 0;
}

int AudioDecoder::decode(DATA_STREAM * inbuf, AVFrame * &pFrame)
{
	return decode(inbuf->_data(), inbuf->size(), inbuf->pts_time(), pFrame);
}

int AudioDecoder::decode(uint8_t* inbuf, uint32_t size, uint64_t pts_time, AVFrame * &pFrame)
{
	int ret = 0; 
	if(NULL == inbuf 
        || size < 0
        || size > AUDIO_DECODER_BUFF_SIZE 
        || NULL == m_deCodecCtx)
	{
		return -1;
	}
	m_decodePacket->data = inbuf;
	m_decodePacket->size = size;

    ret = avcodec_send_packet(m_deCodecCtx, m_decodePacket);
    if (0 == ret)
    {
        ret = avcodec_receive_frame(m_deCodecCtx, pFrame);
    }
    if (ret != 0)
    {
        return ret;
    }
    if (AV_CODEC_ID_PCM_S16LE == m_deCodecCtx->codec_id)
    {
        pFrame->channels = m_deCodecCtx->channels;
        pFrame->channel_layout = m_deCodecCtx->channel_layout;
        pFrame->sample_rate = m_deCodecCtx->sample_rate;
    }

	if(NULL != m_filter_graph)
	{
		AVRational time_base_q={1,AV_TIME_BASE};  
		pFrame->pts = av_rescale_q(pts_time, time_base_q, m_deCodecCtx->time_base);

		ret = av_buffersrc_add_frame(m_buffersrc_ctx, pFrame);
		if (ret != 0)
		{
			std::cout << "av_buffersrc_add_frame error !!!!!, ret: " << ret;
		}
		av_frame_unref(pFrame);
		av_buffersink_get_frame(m_buffersink_ctx, pFrame);
	}

	return ret;
}

void AudioDecoder::init_filter(void)
{
	const AVFilter *abuffer = NULL;
	const AVFilter *volume = NULL;
	const AVFilter *abuffersink = NULL;
    AVFilterContext *volume_ctx = NULL;
	AVDictionary *options_dict = NULL;

	/* Create a new filtergraph, which will contain all the filters. */
	m_filter_graph = avfilter_graph_alloc();

	 /* Create the abuffer filter;
     * it will be used for feeding the data into the graph. */
    abuffer = avfilter_get_by_name("abuffer");

	m_buffersrc_ctx = avfilter_graph_alloc_filter(m_filter_graph, abuffer, "src");
	if(NULL == m_buffersrc_ctx)
	{
		fprintf(stderr, "Could not allocate the abuffer instance.\n");
	}

	//src
	do 
	{
		char args[512] = {};
		ACE_OS::snprintf(args, sizeof(args),
			"time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=%d",
			m_deCodecCtx->time_base.num, m_deCodecCtx->sample_rate, m_deCodecCtx->sample_rate,
			av_get_sample_fmt_name(m_deCodecCtx->sample_fmt),
			m_deCodecCtx->channel_layout);

		avfilter_init_str(m_buffersrc_ctx, args);
	} while (0);
	

	//音量
	do 
	{
		volume = avfilter_get_by_name("volume");
		volume_ctx = avfilter_graph_alloc_filter(m_filter_graph, volume, "volume");
		if(NULL == volume_ctx)
		{
			fprintf(stderr, "Could not allocate the volume instance.\n");
		}
		av_dict_set(&options_dict, "volume", AV_STRINGIFY(1.20), 0);
		avfilter_init_dict(volume_ctx, &options_dict);
	} while (0);
	

	//sink
	do 
	{
		abuffersink = avfilter_get_by_name("abuffersink");
		m_buffersink_ctx = avfilter_graph_alloc_filter(m_filter_graph, abuffersink, "sink");
		if(NULL == m_buffersink_ctx)
		{
			fprintf(stderr, "Could not allocate the aformat instance.\n");
		}
		avfilter_init_str(m_buffersink_ctx, NULL);
	} while (0);
	

	//link
	avfilter_link(m_buffersrc_ctx, 0, volume_ctx, 0);
	avfilter_link(volume_ctx, 0, m_buffersink_ctx, 0);

	//
	avfilter_graph_config(m_filter_graph, NULL);

	if (NULL != options_dict)
	{
		av_dict_free(&options_dict);
	}
	std::cout<<"init filter suc" << std::endl;
}