#include "InterfaceService.h"
#include "HttpServer.h"
#include "filestreamStartHandler.h"
#include "fileStreamStopHandler.h"
#include "fileStreamJumpHandler.h"
#include "fileStreamPauseHandler.h"
#include "fileStreamResumeHandler.h"
#include "fileStreamQueryHandler.h"
#include "realtimeStreamStartHandler.h"
#include "realtimeStreamStopHandler.h"
#include "realtimeStreamQueryHandler.h"
#include "define.h"
#include "router.h"
#include "AppConfig.h"
#include "spdlogging.h"
#include <thread>


InterfaceService::InterfaceService() : IService("Interface service module")
{
    //初始化HttpServer
	m_httpSvr = new HttpServer;
}


InterfaceService::~InterfaceService()
{
    delete m_httpSvr;
    m_httpSvr = NULL;
}

int InterfaceService::open() 
{
    // 初始化路由
    Router r1("/v1/filestream");
    r1.addHandler("/start", new filestreamStartHandler());
    r1.addHandler("/stop", new fileStreamStopHandler());
    r1.addHandler("/jump", new filestreamJumpHandler());
    r1.addHandler("/pause", new fileStreamPauseHandler());
    r1.addHandler("/resume", new fileStreamResumeHandler());
    r1.addHandler("/query", new fileStreamQueryHandler());
    Router r2("/v1/realtimestream");
    r2.addHandler("/start", new realtimeStreamStartHandler());
    r2.addHandler("/stop", new realtimeStreamStopHandler());
    r2.addHandler("/query", new realtimeStreamQueryHandler());

    m_httpSvr->addRouter(&r1);
    m_httpSvr->addRouter(&r2);

    std::thread listenThread(&InterfaceService::httpListen, this);
    listenThread.detach();

    return 0;
};

void InterfaceService::httpListen()
{
    int port = DEFAULT_HTTP_API_PORT;
    if (0 != AppConfig::instance()->getIntParam("service", "port", port))
    {
        port = DEFAULT_HTTP_API_PORT;
    }
    SPDINFO("Start http server, http port: {}", port);
    m_httpSvr->open(port);
}

void InterfaceService::close() 
{ 
	m_httpSvr->close();
	return; 
};