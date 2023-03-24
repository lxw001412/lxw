﻿/**
*
* @file 	realtimeStreamStartHandler.h 
* @author	黄晨阳
* @date		2021.12.2
* @brief
*          启动文件流接口
*
**/
#pragma once

#include "simpleHandler.h"

class realtimeStreamStartHandler :public SimpleHandler
{
public:
    realtimeStreamStartHandler() :SimpleHandler() {}

protected:
    virtual void post(const Json::Value& jsonReq, Json::Value& jsonResp);
};