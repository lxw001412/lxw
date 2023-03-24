/**
* @file audioParserMp3.cpp
* @brief MP3音频帧解析
* @author Bill<pengyouwei@comtom.cn>
* @version 0.1
* @date 2022-10-13
* @description MP3音频帧解析类
*/

#include "audioParserMp3.h"
#include <stdint.h>

// mp3相关类型声明
typedef enum
{
    Version25 = 0,
    VerReserved = 1,
    Version20 = 2,
    Version10 = 3
} EnumMpegVersion;

typedef enum
{
    LayReserved = 0,
    Layer3 = 1,
    Layer2 = 2,
    Layer1 = 3
} EnumMpegLayer;

Mp3AudioParser::Mp3AudioParser() : IAudioParser(),
    m_parseFrameOk(false),
    m_bitRate(0),
    m_sampleRate(0),
    m_frameSize(0),
    m_channelNum(0),
    m_duration(0)
{

}

Mp3AudioParser::~Mp3AudioParser()
{

}

int Mp3AudioParser::ParseFrame(const void* pdata, int length)
{
    static int bitrates[2][3][16] =
    {
        { // MPEG 2 & 2.5
            {0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,0}, // Layer III
            {0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,0}, // Layer II
            {0, 32, 48, 56, 64, 80, 96,112,128,144,160,176,192,224,256,0}  // Layer I
        },
        { // MPEG 1
            {0, 32, 40, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,0}, // Layer III
            {0, 32, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,384,0}, // Layer II
            {0, 32, 64, 96,128,160,192,224,256,288,320,352,384,416,448,0}  // Layer I
        }
    };

    static int frequecies[3][4] =
    {
        {44100, 48000, 32000, 0},	// MPEG 1
        {22050, 24000, 16000, 0},	// MPEG 2
        {11025, 12000, 8000,  0}	// MPEG 2.5	
    };

    static double frameduration[3][4] =
    {
        {26.12245f, 24.0f, 36.0f, 0},	// Layer3
        {26.12245f, 24.0f, 36.0f, 0},	// Layer2		
        {8.707483f,  8.0f, 12.0f, 0}	// Layer1
    };

    m_parseFrameOk = false;

    if (4 > length)
    {
        return -1;
    }

    const uint8_t* pHeader = (const uint8_t*)pdata;

    long nHeader = (pHeader[0] << 24) |
        (pHeader[1] << 16) |
        (pHeader[2] << 8) |
        pHeader[3];

    // Check Sync Word
    if ((nHeader & 0xFFE00000) != 0xFFE00000 || // INVALID Sync Work
        (nHeader & 0x0000FC00) == 0x0000FC00 ||	// Reserved bps & frequency
        (nHeader & 0x0000F000) == 0x00000000)	// Unsoupport free bps
    {
        return -2;
    }

    // Check Version
    EnumMpegVersion eVersion = (EnumMpegVersion)((nHeader & 0x00180000) >> 19);

    // Check Layer
    EnumMpegLayer eLayer = (EnumMpegLayer)((nHeader & 0x00060000) >> 17);
    if (eLayer < Layer3)
    {
        return -2;
    }

    // Bitrate
    int i;
    i = (nHeader & 0x0000F000) >> 12;

    if (eVersion == Version10)
    {
        m_bitRate = bitrates[1][eLayer - Layer3][i];
    }
    else
    {
        m_bitRate = bitrates[0][eLayer - Layer3][i];
    }

    // SampleRate
    i = (nHeader & 0x00000C00) >> 10;
    switch (eVersion)
    {
    case Version10:
        m_sampleRate = frequecies[0][i];
        break;

    case Version20:
        m_sampleRate = frequecies[1][i];
        break;

    case Version25:
        m_sampleRate = frequecies[2][i];
        break;

    default:
        m_sampleRate = 0;
        break;
    }

    if (m_sampleRate == 0 || m_bitRate == 0)
        return false;

    // Play Duration
    m_duration = frameduration[eLayer - Layer3][i];

    if ((eVersion == Version25/* || eVersion == Version20*/)
        && (eLayer == Layer3))
    {
        m_duration *= 2;
    }

    // Frame Length
    i = (nHeader & 0x00000200) >> 9;
    if (eLayer == Layer1)
    {
        m_frameSize = (12000 * m_bitRate / m_sampleRate + i) * 4;
    }
    else
    {
        if (eVersion == Version10)
            m_frameSize = 144000 * m_bitRate / m_sampleRate + i;
        else if (eLayer == Layer3)
            m_frameSize = 144000 * m_bitRate / m_sampleRate / 2 + i;
        else
            m_frameSize = 144000 * m_bitRate / m_sampleRate + i;
    }

    // Channel Mode
    i = (nHeader & 0x000000C0) >> 6;
    m_channelNum = (i == 3) ? 1 : 2;

    if (0 == m_frameSize)
    {
        return -2;
    }

    m_parseFrameOk = true;
    return 0;
}
