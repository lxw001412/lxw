/**
 * @file audioParser.cpp
 * @brief 音频帧解析
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2022-10-13
 * @description 音频帧解析接口类定义
 */

#include "audioParser.h"
#include "audioParserAac.h"
#include "audioParserMp3.h"
#include <stdio.h>

IAudioParser* IAudioParser::CreateAudioParser(EnumFrameType type)
{
    switch (type)
    {
    case FrameTypeMP3:
        return new Mp3AudioParser();
        break;
    case FrameTypeAAC:
        return new AacAudioParser();
        break;
    default:
        break;
    }
    return NULL;
}