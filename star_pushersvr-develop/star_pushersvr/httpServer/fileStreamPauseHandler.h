/**
*
* @file 	fileStreamPauseHandler.h
* @author	黄晨阳
* @date		2021.12.2
* @brief
*          暂停文件流
*
**/
#pragma once
#include "simpleHandler.h"

class fileStreamPauseHandler : public SimpleHandler
{
public:
    fileStreamPauseHandler() :SimpleHandler() {}

protected:
    virtual void post(const Json::Value& jsonReq, Json::Value& jsonResp);
};