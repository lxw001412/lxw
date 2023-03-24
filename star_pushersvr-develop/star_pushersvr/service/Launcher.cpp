#include "Launcher.h"
#include "InterfaceService.h"
#include "HttpClientService.h"
#include "streamMod.h"
#include "spdlogging.h"
#include "ConfigMod.h"
#include "AppConfig.h"
#include "codecDef.h"

#include <ace/Reactor.h>
#include <ace/Get_Opt.h>
#include <iostream>

#define MACRO_DEF_STR(m) MACRO_TEMP(m)
#define MACRO_TEMP(m) #m

int Launcher::init(int argc, char *argv[])
{
	ACE_Get_Opt get_opt(argc, argv, ACE_TEXT("vhc:"));
	int option = 0;
	while ((option = get_opt()) != EOF)
	{
		switch (option)
		{
		case -1:
		{
			m_Help = true;
			return 0;
			break;
		}
		case 'v':
		{
			m_Ver = true;
			return 0;
			break;
		}
		case 'h':
		{
			m_Help = true;
			return 0;
			break;
		}
		case 'c':
		{
			if (get_opt.opt_arg() == NULL)
			{
				m_Help = true;
			return 0;
                break;
			}
			//获取配置文件路径
			m_ConfigPath = get_opt.opt_arg();
			break;
		}
		default:
		{
			m_Help = true;
			return 0;
			break;
		}
		}
	};

    IService* service = NULL;
    int rc = 0;

    // 配置模块
    service = new ConfigMod();
    rc = static_cast<ConfigMod*>(service)->init(m_ConfigPath);
    if (0 != rc)
    {
        SPDERROR("[launcher] Init {} failed, rc: {}", service->name(), rc);
        return 1;
    }
    SPDINFO("[launcher] Init: {}", service->name());
    m_service.push_back(service);

    // 初始化日志
    spdlogging::logParam param;
    param.fileLogPath = "star_broadcastsvr.log";
    param.fileLogNum = 1;
    param.fileLogSize = 50;
    param.fileLogLevel = "debug";
    param.enableConsoleLog = true;
    param.consoleLogLevel = "info";
    AppConfig::instance()->getStringParam("log", "logLevel", param.fileLogLevel);
    AppConfig::instance()->getStringParam("log", "file", param.fileLogPath);
    AppConfig::instance()->getIntParam("log", "fileNumber", param.fileLogNum);
    AppConfig::instance()->getIntParam("log", "fileSize", param.fileLogSize);
    AppConfig::instance()->getBoolParam("log", "enableConsoleLog", param.enableConsoleLog);
    AppConfig::instance()->getStringParam("log", "consoleLogLevel", param.consoleLogLevel);
    // 初始化日志
    spdlogging::init(param);

    SPDINFO("[launcher] version-data <{}------{}>\n", MACRO_DEF_STR(VERSION), MACRO_DEF_STR(DATE));

    // 编解码模块初始化
    CodecInit();

    // 推流模块
    service = new StreamMod();
    rc = service->init();
    if (0 != rc)
    {
        SPDERROR("[launcher] Init {} failed, rc: {}", service->name(), rc);
        return 1;
    }
    SPDINFO("[launcher] Init: {}", service->name());
    m_service.push_back(service);

    // 接口模块
    service = new InterfaceService();
    rc = service->init();
    if (0 != rc)
    {
        SPDERROR("[launcher] Init {} failed, rc: {}", service->name(), rc);
        return 1;
    }
    SPDINFO("[launcher] Init: {}", service->name());
    m_service.push_back(service);

    // HTTP Client模块
    service = new HttpClientService();
    rc = service->init();
    if (0 != rc)
    {
        SPDERROR("[launcher] Init {} failed, rc: {}", service->name(), rc);
        return 1;
    }
    SPDINFO("[launcher] Init: {}", service->name());
    m_service.push_back(service);

    return 0;
}

void Launcher::finit()
{
	for (auto service : m_service)
	{
		if (service != NULL)
		{
			service->finit();
            SPDINFO("[launcher] Finit: {}", service->name());
			delete service;
			service = NULL;
		}
	}
	m_service.clear();

    // 编解码模块清理
    CodecCleanup();
}

int Launcher::open()
{
	if (m_Help)
	{
		printUsage();
		return 0;
	}

	if (m_Ver)
	{
        printVersion();
		return 0;
	}
    
    int rc = 0;
    for (auto service : m_service)
    {
        if (service != NULL)
        {
            rc = service->open();
            if (0 != rc)
            {
                SPDERROR("[launcher] Open {} failed, rc: {}", service->name(), rc);
                return 1;
            }
            SPDINFO("[launcher] Open: {}", service->name());
        }
    }

    ACE_Reactor *reactor = ACE_Reactor::instance();
    reactor->register_handler(1, this);
    reactor->register_handler(SIGINT, this);

    m_ThreadSign = true;
    while (m_ThreadSign)
    {
        ACE_Time_Value timeout(0, 100 * 1000);  // 100 ms
        reactor->run_reactor_event_loop(timeout);
    } 
    reactor->remove_handler(1, NULL, NULL, -1);
    reactor->remove_handler(SIGINT, NULL, NULL, -1);
    return 0;
}

void Launcher::close()
{
	for (auto service : m_service)
	{
		service->close();
        SPDINFO("[launcher] Close: {}", service->name());
	}

}

void Launcher::printUsage()
{
	std::cout << "Usage: " << std::endl
		<< APPLICATION << " [-c config_file_path][-v][-h]" << std::endl;
}

void Launcher::printVersion()
{
	printf("version-data <%s------%s>\n", MACRO_DEF_STR(VERSION), MACRO_DEF_STR(DATE));
}

int Launcher::handle_signal(int signum, siginfo_t * /*= 0*/, ucontext_t * /*= 0*/)
{
    if (signum == 1)
    {
        reloadConfig();
    }
    else
    {
        m_ThreadSign = false;
    }
    return 0;
}

void Launcher::reloadConfig()
{
    SPDINFO("[launcher] ======== reload config");
    AppConfig::instance()->load(m_ConfigPath);

    // 重新初始化日志
    spdlogging::logParam param;
    param.fileLogPath = "star_broadcastsvr.log";
    param.fileLogNum = 1;
    param.fileLogSize = 50;
    param.fileLogLevel = "debug";
    param.enableConsoleLog = true;
    param.consoleLogLevel = "info";
    AppConfig::instance()->getStringParam("log", "logLevel", param.fileLogLevel);
    AppConfig::instance()->getStringParam("log", "file", param.fileLogPath);
    AppConfig::instance()->getIntParam("log", "fileNumber", param.fileLogNum);
    AppConfig::instance()->getIntParam("log", "fileSize", param.fileLogSize);
    AppConfig::instance()->getBoolParam("log", "enableConsoleLog", param.enableConsoleLog);
    AppConfig::instance()->getStringParam("log", "consoleLogLevel", param.consoleLogLevel);
    // 初始化日志
    spdlogging::init(param);

    SPDINFO("[launcher] ======== reinit log");
}