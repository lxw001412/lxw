#include "protocol.h"
#include "productModelImp.h"
#include "commandModelImp.h"
#include "spdlogging.h"

using namespace star_protocol;


int addProductModel(const Json::Value &jsonRoot)
{
	return star_protocol::productModelRepo::instance()->addProductModel(jsonRoot);
}

void destroyProductModelRepo()
{
    star_protocol::productModelRepo::destory();
}

int addCommandModel(const Json::Value &jsonRoot)
{
    return star_protocol::commandModelRepo::instance()->addCommandModel(jsonRoot);
}

void destroyCommandModelRepo()
{
    star_protocol::commandModelRepo::destory();
}

int initProtocolLog(const char* logLevel,
    const char* logPath,
    int fileNumber,
    int fileSize,
    bool enableConsoleLog)
{
    star_protocol::logParam logParam;
    logParam.fileLogLevel = logLevel; 
    logParam.fileLogPath = logPath;
    logParam.fileLogNum = fileNumber;
    logParam.fileLogSize = fileSize;
    logParam.enableConsoleLog = enableConsoleLog;
    logParam.consoleLogLevel = logLevel;
    return star_protocol::initSpdLog(logParam);
}

void finiProtocolLog()
{
    star_protocol::finiSpdLog();
}
