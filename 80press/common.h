#pragma once
#include "Unicode.h"
#include "termMessage.h"
#include "termPackage.h"
#include "CHandle.h"
#include "CLog.h"
#include "ace/Reactor.h"
#include "ReactorSvr.h"
#include "ace/Time_Value.h"
#define IP "192.168.112.224"
#define PORT "7900"
#define RTMP_ADDR "rtmp://113.240.243.236/live?vhost=live.test/test-0309"
#define LOCALADDR "0.0.0.0:0"
#define EXAMINATION 10000
#define NORMAL      1000
typedef struct ModInfo
{
	int  m_hart;          //拉流心跳
	int  m_size;          //请求数量
	int  m_examhart;      //考试心跳
	bool m_isExam;        //是否开启考试模式
	ReactorSvr* m_network;
	std::string m_TargeAddr; //拉流地址
};