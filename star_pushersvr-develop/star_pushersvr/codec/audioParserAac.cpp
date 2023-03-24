/**
* @file audioParserAac.cpp
* @brief AAC音频帧解析
* @author Bill<pengyouwei@comtom.cn>
* @version 0.1
* @date 2022-10-13
* @description AAC音频帧解析类
*/

#include "audioParserAac.h"

typedef struct _AdtsHeader
{
    unsigned int nSyncWord;
    unsigned int nId;
    unsigned int nLayer;
    unsigned int nProtectionAbsent;
    unsigned int nProfile;
    unsigned int nSfIndex;
    unsigned int nPrivateBit;
    unsigned int nChannelConfiguration;
    unsigned int nOriginal;
    unsigned int nHome;

    unsigned int nCopyrightIdentificationBit;
    unsigned int nCopyrigthIdentificationStart;
    unsigned int nAacFrameLength;
    unsigned int nAdtsBufferFullness;

    unsigned int nNoRawDataBlocksInFrame;
} AdtsHeader;

// 解析ATDS头
static int getAacHeader(const char *_pBuffer, AdtsHeader *_header);

// 取音频帧采样率
static int GetAdtsSampleRate(int index);

// 取音频帧时长
static double GetAdtsDuration(int profile, int sampleRateIndex);

#define ADTS_HEADER_SIZE_MIN    7

// 解析ATDS头
static int getAacHeader(const char *_pBuffer, AdtsHeader *_header)
{
    // headers begin with FFFxxxxx...
    if ((unsigned char)_pBuffer[0] == 0xff && (((unsigned char)_pBuffer[1] & 0xf0) == 0xf0)) {
        _header->nSyncWord = (_pBuffer[0] << 4) | (_pBuffer[1] >> 4);
        _header->nId = ((unsigned int)_pBuffer[1] & 0x08) >> 3;
        _header->nLayer = ((unsigned int)_pBuffer[1] & 0x06) >> 1;
        _header->nProtectionAbsent = (unsigned int)_pBuffer[1] & 0x01;
        _header->nProfile = ((unsigned int)_pBuffer[2] & 0xc0) >> 6;
        _header->nSfIndex = ((unsigned int)_pBuffer[2] & 0x3c) >> 2;
        _header->nPrivateBit = ((unsigned int)_pBuffer[2] & 0x02) >> 1;
        _header->nChannelConfiguration = ((((unsigned int)_pBuffer[2] & 0x01) << 2) | (((unsigned int)_pBuffer[3] & 0xc0) >> 6));
        _header->nOriginal = ((unsigned int)_pBuffer[3] & 0x20) >> 5;
        _header->nHome = ((unsigned int)_pBuffer[3] & 0x10) >> 4;
        _header->nCopyrightIdentificationBit = ((unsigned int)_pBuffer[3] & 0x08) >> 3;
        _header->nCopyrigthIdentificationStart = (unsigned int)_pBuffer[3] & 0x04 >> 2;
        _header->nAacFrameLength = (((((unsigned int)_pBuffer[3]) & 0x03) << 11) |
            (((unsigned int)_pBuffer[4] & 0xFF) << 3) |
            ((unsigned int)_pBuffer[5] & 0xE0) >> 5);
        _header->nAdtsBufferFullness = (((unsigned int)_pBuffer[5] & 0x1f) << 6 | ((unsigned int)_pBuffer[6] & 0xfc) >> 2);
        _header->nNoRawDataBlocksInFrame = ((unsigned int)_pBuffer[6] & 0x03);
        return 0;
    }
    else {
        return -1;
    }
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

// 取音频帧采样率
static int GetAdtsSampleRate(int index)
{
    static int table[ASC_SF_RESERVED_1] = {
        96000,
        88200,
        64000,
        48000,
        44100,
        32000,
        24000,
        22050,
        16000,
        12000,
        11025,
        8000,
        7350
    };
    if (index >= ASC_SF_RESERVED_1)
    {
        return 0;
    }
    return table[index];
}

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

AacAudioParser::AacAudioParser() : IAudioParser(),
m_parseFrameOk(false),
m_bitRate(0),
m_sampleRate(0),
m_frameSize(0),
m_channelNum(0),
m_duration(0)
{

}

AacAudioParser::~AacAudioParser()
{

}

int AacAudioParser::ParseFrame(const void* pdata, int length)
{
    m_parseFrameOk = false;
    if (ADTS_HEADER_SIZE_MIN > length)
    {
        return -1;
    }
    AdtsHeader adtsHeader;
    if (0 != getAacHeader((const char *)pdata, &adtsHeader))
    {
        return -2;
    }
    m_parseFrameOk = true;
    m_bitRate = -2;
    m_sampleRate = GetAdtsSampleRate(adtsHeader.nSfIndex);
    m_frameSize = adtsHeader.nAacFrameLength;
    m_channelNum = adtsHeader.nChannelConfiguration;
    m_duration = GetAdtsDuration(adtsHeader.nProfile, adtsHeader.nSfIndex);

    return 0;
}
