
#pragma once

#include "HttpClient.h"
#include "HttpRequest.h"
#include "HttpClientManage.h"

class Module_Interface
{
public:
    static int sendStreamProgress(HttpRequest* msg)
    {
        HttpClient* hc = HTTPCLIENTMANAGE::instance()->getStreamProgress();
        if (NULL == hc)
        {
            return 1;
        }
        return hc->push(msg);
    };
    static int sendStreamStop(HttpRequest* msg)
    {
        HttpClient* hc = HTTPCLIENTMANAGE::instance()->getStreamStop();
        if (NULL == hc)
        {
            return 1;
        }
        return hc->push(msg);
    };
};
