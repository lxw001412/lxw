/**
*
* @file 	HttpClientManage.h
* @author	黄晨阳
* @date		2021.12.1
* @brief
* 该类为http客户端管理类
**/

#pragma once
#include <ace/Singleton.h>
#include "HttpClient.h"
class HttpClientManage
{
public:
    /*
   * @brief 实例化客户端对象，并开启对应线程
   *
   */
    int open();
    
    /*
   * @brief  销毁客户端对象，并关闭对应线程
   *
   */
    int close();

    /*
   * @brief  获取StreamProgress对象指针
   *
   */
    HttpClient* getStreamProgress() { return m_streamProgress; }

    /*
   * @brief  获取streamStop对象指针
   *
   */
    HttpClient* getStreamStop() { return m_streamStop; }
private:
    HttpClient* m_streamProgress = NULL;
    HttpClient* m_streamStop = NULL;
private:
    friend class ACE_Singleton<HttpClientManage, ACE_Thread_Mutex>;
};
typedef ACE_Singleton<HttpClientManage, ACE_Thread_Mutex> HTTPCLIENTMANAGE;