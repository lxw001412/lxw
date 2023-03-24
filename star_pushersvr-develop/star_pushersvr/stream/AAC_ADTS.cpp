#include "AAC_ADTS.h"
#include <string.h>

static const uint16_t static_syncword = 0;
static const uint16_t static_ID =static_syncword + 1;
static const uint16_t static_profile = static_ID + 1;
static const uint16_t static_channel_configuration = static_profile + 1;
static const uint16_t static_adts_buffer_fullness = static_channel_configuration + 2;

static double GetAdtsDuration(int profile, int sampleRateIndex);

AAC_ADTS::AAC_ADTS(void)
{
	m_Data = new unsigned char[10];
	memset(m_Data, 0, 10);
	syncword(0xfff);
	ID(1);
	profile(1);
	adts_buffer_fullness(0x7ff);
    m_profile = 1;
    m_sampleRateIndex = 4;
}

AAC_ADTS::~AAC_ADTS(void)
{
	delete []m_Data;
}


void AAC_ADTS::syncword(uint16_t ivalue)
{
	uint8_t tmp = (ivalue&0xff0)>>4;
	m_Data[static_syncword] = tmp;
	tmp = ((ivalue&0xf)<<4)|(m_Data[static_syncword+1]&0xf);
	m_Data[static_syncword+1] = tmp;
}

void AAC_ADTS::ID(uint16_t ivalue)
{
	uint8_t tmp = (ivalue&0x01)|(m_Data[static_ID]&0xf7);
	m_Data[static_ID] = tmp;
}

void AAC_ADTS::layer(uint16_t ivalue)
{

}

void AAC_ADTS::protection_absent(uint16_t ivalue)
{
	uint8_t tmp = (ivalue&0x01)|(m_Data[static_ID]&0xfc);
	m_Data[static_ID] = tmp;
}

void AAC_ADTS::profile(uint16_t ivalue)
{
	uint8_t tmp = ((ivalue&0x3)<<6)|(m_Data[static_profile]&0x3f);
	m_Data[static_profile] = tmp;
}

void AAC_ADTS::sample_rate_index(uint32_t ivalue)
{
	uint8_t tmp = 0;
	if(96000 == ivalue)
	{
		tmp = 0;
	}
	else if(88200 == ivalue)
	{
		tmp = 1;
	}
	else if(64000 == ivalue)
	{
		tmp = 2;
	}
	else if(48000 == ivalue)
	{
		tmp = 3;
	}
	else if(44100 == ivalue)
	{
		tmp = 4;
	}
	else if(32000  == ivalue)
	{
		tmp = 5;
	}
	else if(24000  == ivalue)
	{
		tmp = 6;
	}
	else if(22050 == ivalue)
	{
		tmp = 7;
	}
	else if(16000 == ivalue)
	{
		tmp = 8;
	}
	else if(12000 == ivalue)
	{
		tmp = 9;
	}
	else if(11025 == ivalue)
	{
		tmp = 10;
	}
	else if(8000 == ivalue)
	{
		tmp = 11;
	}
	else if(7350 == ivalue)
	{
		tmp = 12;
	}
	else
	{
		tmp = 13;
	}

    m_sampleRateIndex = tmp;

	tmp = (tmp<<2)|(m_Data[static_profile]&0xc3);
	m_Data[static_profile] = tmp;
}

void AAC_ADTS::private_bit(uint16_t ivalue)
{

}

void AAC_ADTS::channel_configuration(uint16_t ivalue)
{
	uint8_t tmp = ((ivalue&0x3)<<6)|(m_Data[static_channel_configuration]&0x3f);
	m_Data[static_channel_configuration] = tmp;
}

void AAC_ADTS::original_copy(uint16_t ivalue)
{

}

void AAC_ADTS::home(uint16_t ivalue)
{

}

void AAC_ADTS::copyright_identification_bit(uint16_t ivalue)
{

}

void AAC_ADTS::copyright_identification_start(uint16_t ivalue)
{

}

void AAC_ADTS::aac_frame_length(uint16_t ivalue)
{
	int pos = static_channel_configuration;
	uint8_t tmp = ((ivalue>>11)&0x3)|(m_Data[pos]&0xfc);
	m_Data[pos] = tmp;
	pos++;
	tmp = ((ivalue>>3)&0xff);
	m_Data[pos] = tmp;
	pos++;
	tmp = ((ivalue&0x7)<<5)|(m_Data[pos]&0x1f);
	m_Data[pos] = tmp;

}

void AAC_ADTS::adts_buffer_fullness(uint16_t ivalue)
{
	int pos = static_adts_buffer_fullness;
	uint8_t tmp = ((ivalue>>6)&0x1f)|(m_Data[pos]&0xE0);
	m_Data[pos] = tmp;
	pos++;
	tmp = ((ivalue&0x3f)<<2)|(m_Data[pos]&0x3);
	m_Data[pos] = tmp;
}

void AAC_ADTS::number_of_raw_data_blocks_in_frame(uint16_t ivalue)
{

}

double AAC_ADTS::frame_duration()
{
    return GetAdtsDuration(m_profile, m_sampleRateIndex);
}

//
// sampling frequencies
//
#define ASC_SF_96000      0
#define ASC_SF_88200      1
#define ASC_SF_64000      2
#define ASC_SF_48000      3
#define ASC_SF_44100      4
#define ASC_SF_32000      5
#define ASC_SF_24000      6
#define ASC_SF_22050      7
#define ASC_SF_16000      8
#define ASC_SF_12000      9
#define ASC_SF_11025      10
#define ASC_SF_8000       11
#define ASC_SF_7350       12
#define ASC_SF_RESERVED_1 13
#define ASC_SF_RESERVED_2 14
#define ASC_SF_CUSTOM     15

static double GetAdtsDuration(int profile, int sampleRateIndex)
{
    if (sampleRateIndex < 0
        || sampleRateIndex >= ASC_SF_RESERVED_1
        || profile < 0
        || profile >= 4)
    {
        return 0;
    }
    static double frameduration[4][ASC_SF_RESERVED_1] =
    {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// profile main 不支持
        {10.666666667f, 11.609977324f, 16.0f, 21.333333333f, 23.219954649f, 0, 0, 46.439909297f, 0, 0, 92.879818594f, 0, 0},	// profile LC
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// profile ssr 不支持
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}	    // profile unknown 不支持
    };
    return frameduration[profile][sampleRateIndex];
}
