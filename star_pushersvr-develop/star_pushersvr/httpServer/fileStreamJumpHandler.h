/**
*
* @file 	filestreamJumpHandler.h
* @author	黄晨阳
* @date		2021.12.2
* @brief
*          文件流播放进度控制
*
**/
#pragma once
#include "simpleHandler.h"
class StreamIn;
class filestreamJumpHandler : public SimpleHandler
{
public:
    filestreamJumpHandler() :SimpleHandler() {}

protected:
    virtual void post(const Json::Value& jsonReq, Json::Value& jsonResp);
private:
	StreamIn* m_streamIn;
};