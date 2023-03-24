/**
*
* @file 	fileStreamStopHandler.h
* @author	黄晨阳
* @date		2021.12.3
* @brief
*          停止文件流接口
*
**/
#pragma once

#include "simpleHandler.h"

class fileStreamStopHandler : public SimpleHandler
{
public:
    fileStreamStopHandler() :SimpleHandler() {}

protected:
    virtual void post(const Json::Value& jsonReq, Json::Value& jsonResp);
};
