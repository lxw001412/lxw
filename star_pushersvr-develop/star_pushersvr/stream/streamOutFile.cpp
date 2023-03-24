/**
 * @file streamOutFile.cpp
 * @brief 文件流输出类
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-11-29
 * @description 文件流输出类
 */

#include "streamOutFile.h"

StreamOutFile::StreamOutFile() : m_fp(NULL), m_codec(AC_UNSUPPORT)
{

}

StreamOutFile::~StreamOutFile()
{
    close();
}

int StreamOutFile::open(const std::string &id, const std::string &file, AudioCodec_t codec, int channel, int sampleRate, int sampleBits)
{
    m_codec = codec;
    m_id = id;
    m_fp = fopen(file.c_str(), "wb");
    if (m_fp != NULL)
    {
        return 0;
    }
    return -1;
}

int StreamOutFile::close()
{
    if (m_fp != NULL)
    {
        fclose(m_fp);
        m_fp = NULL;
    }
    return 0;
}

int StreamOutFile::writeData(FrameData_t &frameData)
{
    fwrite(frameData.data, 1, frameData.length, m_fp);
    return 0;
}