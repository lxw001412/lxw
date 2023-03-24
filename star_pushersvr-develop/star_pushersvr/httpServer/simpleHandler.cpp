#include "simpleHandler.h"
#include "httpServerError.h"
#include "../common/utils.h"


void SimpleHandler::get(const Request & req, Response & res)
{
	Json::Value jsonReq, jsonResp;
	std::string resstring;
	for (auto it = req.params.begin(); it != req.params.end(); it++)
	{
		jsonReq[it->first] = it->second;
	}
	get(jsonReq, jsonResp);
	json2str(jsonResp, resstring);
	res.set_content(resstring, "application/json");
}

void SimpleHandler::post(const Request & req, Response & res)
{
	Json::Value jsonReq, jsonResp;
	if (reqParamProcess(req, jsonReq, jsonResp) != -1)
	{
		post(jsonReq, jsonResp);
	}
	std::string resstring;
	json2str(jsonResp, resstring);
	res.set_content(resstring, "application/json");
}

void SimpleHandler::put(const Request & req, Response & res)
{
	Json::Value jsonReq, jsonResp;
	if (reqParamProcess(req, jsonReq, jsonResp) != -1)
	{
		put(jsonReq, jsonResp);
	}
	std::string resstring;
	json2str(jsonResp, resstring);
	res.set_content(resstring, "application/json");
}

void SimpleHandler::del(const Request & req, Response & res)
{
	Json::Value jsonReq, jsonResp;
	if (reqParamProcess(req, jsonReq, jsonResp) != -1)
	{
		del(jsonReq, jsonResp);
	}
	std::string resstring;
	json2str(jsonResp, resstring);
	res.set_content(resstring, "application/json");
}

void SimpleHandler::get(const Json::Value & jsonReq, Json::Value & jsonResp)
{
	jsonResp["code"] = PUSHERSVR_ERROR_INVALID_METHOD;
	jsonResp["msg"] = "method unsuported";
	jsonResp["data"] = {};
}

void SimpleHandler::post(const Json::Value & jsonReq, Json::Value & jsonResp)
{
	jsonResp["code"] = PUSHERSVR_ERROR_INVALID_METHOD;
	jsonResp["msg"] = "method unsuported";
	jsonResp["data"] = {};
}

void SimpleHandler::del(const Json::Value & jsonReq, Json::Value & jsonResp)
{
	jsonResp["code"] = PUSHERSVR_ERROR_INVALID_METHOD;
	jsonResp["msg"] = "method unsuported";
	jsonResp["data"] = {};
}

void SimpleHandler::put(const Json::Value & jsonReq, Json::Value & jsonResp)
{
	jsonResp["code"] = PUSHERSVR_ERROR_INVALID_METHOD;
	jsonResp["msg"] = "method unsuported";
	jsonResp["data"] = {};
}

int SimpleHandler::reqParamProcess(const Request & req, Json::Value & jsonReq, Json::Value & jsonResp)
{
	if (0 != str2json(req.body, jsonReq))
	{
		Json::Value resjson;
		Json::FastWriter writer;
		jsonResp["code"] = PUSHERSVR_ERROR_INVALID_METHOD;
		jsonResp["msg"] = "request body json parse failed";
		jsonResp["data"] = {};
		return -1;
	}
	return 0;
}
