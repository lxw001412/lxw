#pragma once
#include "Ihandler.h"
#include "json/json.h"

class SimpleHandler :
	public IHandler
{
public:
	virtual void get(const Json::Value& jsonReq, Json::Value& jsonResp);

	virtual void post(const Json::Value& jsonReq, Json::Value& jsonResp);

	virtual void del(const Json::Value& jsonReq, Json::Value& jsonResp);

	virtual void put(const Json::Value& jsonReq, Json::Value& jsonResp);

protected:
    virtual void get(const Request & req, Response & res);

    virtual void post(const Request & req, Response & res);

    virtual void put(const Request & req, Response & res);

    virtual void del(const Request & req, Response & res);

private:
	int  reqParamProcess(const Request & req, Json::Value& jsonReq, Json::Value& jsonResp);

};

