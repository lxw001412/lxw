#include "HttpClient.h"

// 缓存的回调数据最大数目
#define MAXMSGSIZE 10240

HttpClient::HttpClient()
{
    m_runing = false;
}

HttpClient::~HttpClient()
{
    m_runing = false;
    ACE_Thread::join(m_threadhandle);
}

int HttpClient::open()
{
    m_runing = true;

    int res = ACE_Thread::spawn((ACE_THR_FUNC)threadfunc, this, THR_JOINABLE | THR_NEW_LWP,
        &m_threadid, &m_threadhandle);
    if (res != 0)
    {
        return 2;
    }
    return 0;
}

void HttpClient::close()
{
    m_runing = false;
    ACE_Thread::join(m_threadhandle);
}

int HttpClient::push(HttpRequest* msg)
{
    if (m_msgQue.size() >= MAXMSGSIZE)
    {
        HttpRequest* discardMsg;
        m_msgQue.pop(discardMsg);
        HttpRequest::destroy(discardMsg);
    }
    return  m_msgQue.push(msg);;
}

int HttpClient::send2Dest(HttpRequest* msg)
{
    std::string resp;
    msg->action(resp);
    return 0;
}


int HttpClient::threadfunc(void* obj)
{
    HttpClient* pthis = static_cast<HttpClient*>(obj);
    while (pthis->m_runing)
    {
        HttpRequest* send;
       int resp = pthis->m_msgQue.pop(send , 3000);
       if (resp == 0)
       {
           pthis->send2Dest(send);
           HttpRequest::destroy(send);
       }
    }
    return 0;
}