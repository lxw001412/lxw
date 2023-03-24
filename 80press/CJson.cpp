#include "CJson.h"

JsonData::JsonData()
{
	m_msgid = 1;
}

void JsonData::Init(std::string sn, std::string addr, int streamid , bool Start)
{

	if(Start)           //拉流请求
	{
		if (addr.empty() || sn.empty())return;
		m_root.clear();
		m_root["qos"] = 1;
		m_root["ack"] = 0;
		m_root["sn"] = sn.c_str();
		m_root["msgid"] = m_msgid++;
		m_root["topic"] = 2;
		m_root["cmd"] = 3;
		m_root["msg"]["url"] = addr.c_str();
		m_root["msg"]["prestreamid"] = 0;
	}
	else if(!Start)          //停止拉流
	{
		if (sn.empty())return;
		m_root.clear();
		m_root["cmd"] = 4;
		m_root["qos"] = 1;
		m_root["ack"] = 0;
		m_root["sn"] = sn.c_str();
		m_root["msgid"] = m_msgid++;
		m_root["topic"] = 2;
		m_root["msg"]["streamid"] = streamid;
	}
}

JsonData::~JsonData()
{
	m_root.clear();
}

std::string JsonData::Dump()
{
	std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter()); //从字符串中输出到Json文件
	return Json::writeString(writerBuilder, m_root);
}

JsonData & JsonData::operator=(const Json::Value & root)
{
	m_root.clear();
	m_root = root;
	return *this;
}
