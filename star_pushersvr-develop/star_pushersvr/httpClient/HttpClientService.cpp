#include "HttpClientService.h"
#include "HttpClientManage.h"


HttpClientService::HttpClientService() : IService("Interface service module")
{
}


HttpClientService::~HttpClientService()
{
}

int HttpClientService::open() 
{
    int res = HTTPCLIENTMANAGE::instance()->open();

    return res;
};

void HttpClientService::close() 
{ 
    HTTPCLIENTMANAGE::instance()->close();
    HTTPCLIENTMANAGE::close();
	return; 
};