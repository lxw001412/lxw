#pragma once
#include <string>
#include "json/json.h"
#include "common.h"
class JsonData
{
public:
	JsonData();
	void Init(std::string sn,std::string addr,int streamid = -1,bool Start = true);
	~JsonData();
	std::string Dump();
	operator Json::Value() { if(!m_root.isNull())return m_root;}
	JsonData& operator = (const Json::Value& root);
public:
	int m_msgid;
	Json::Value m_root;
	Json::StreamWriterBuilder writerBuilder;
};

