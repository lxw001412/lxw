/**
*
* @file 	fileStreamResumeHandler.h
* @author	黄晨阳
* @date		2021.12.2
* @brief
*          恢复文件流
*
**/
#pragma once

#include "simpleHandler.h"

class fileStreamResumeHandler : public SimpleHandler
{
public:
    fileStreamResumeHandler() :SimpleHandler() {}

protected:
    virtual void post(const Json::Value& jsonReq, Json::Value& jsonResp);
};