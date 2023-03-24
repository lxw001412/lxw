/**
*
* @file HttpClient.h
* @brief  负责执行Http请求
*         主要用于各模块回调应用对应接
* @author    chenjie
* @version 0.0.1
* @date        2021.11.30
* @note  取消单例
**/


#pragma once

#include "messageQueue.h"
#include "HttpRequest.h"
#include <ace/Thread.h>

class HttpClient
{
public:
    HttpClient();
    ~HttpClient();
    /*
    * @brief 开启线程
    *
    */
    int open();

    /*
    * @brief 关闭线程
    *
    */
    void close();


    int push(HttpRequest* msg);
private:
    /*
    * @brief 线程负责从队列中获取Http Request，
    * 并执行；如果队列为空，则继续等待消息插入；
    *
    */

    static int threadfunc(void* obj);

    int send2Dest(HttpRequest* msg);

private:

    MessageQueue<HttpRequest*> m_msgQue;
    volatile bool m_runing;

    ACE_thread_t m_threadid;
    ACE_hthread_t m_threadhandle;
};
