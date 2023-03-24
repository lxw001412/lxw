/**
 * @file streamMod.cpp
 * @brief 推流模块接口
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-11-29
 * @description 继承服务模块接口
 */

#include "streamMod.h"
#include "streamModInterface.h"
#include "streamIn.h"
#include "tickcount.h"
#include "spdlogging.h"

StreamMod::StreamMod() : IService("Stream module")
{

}

StreamMod::~StreamMod()
{

}

int StreamMod::init()
{
    srand((int)GetUtcMsCount());

    int rc = ffmpegInit();
    if (0 != rc)
    {
        SPDERROR("[stream-mod] ffmpeg init failed");
        return rc;
    }

    rc = StreamModInterface::init();

    return rc;
}

void StreamMod::finit()
{
    ffmpegFini();

    StreamModInterface::fini();
}

int StreamMod::open()
{
    return StreamModInterface::open();
}

void StreamMod::close()
{
    StreamModInterface::close();
}