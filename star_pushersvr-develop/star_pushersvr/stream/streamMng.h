/**
 * @file streamMng.h
 * @brief 推流对象管理
 * @author Bill<pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-11-30
 * @description 推流对象管理
 */

#pragma once

#include "baseStream.h"
#include "streamDef.h"
#include "tickcount.h"
#include <memory>
#include <map>
#include <string>
#include <mutex>
#include <thread>
#include <chrono>

template <typename  T>
class StreamMng
{
public:
    StreamMng();
    virtual ~StreamMng();

    /**
     * @brief 启动工作线程
     *
     * @return 0 成功，其它失败
     */
    int open();

    /**
     * @brief 停止工作线程
     *
     * @return 0 成功，其它失败
     */
    int close();

    /**
     * @brief 创建推流对象
     *
     * @param id 流ID
     *
     * @return 推流对象智能指针
     */
    std::shared_ptr<StreamBase> createStream(const std::string &id);

    /**
     * @brief 获取推流对象
     *
     * @param id 流ID
     *
     * @return 推流对象智能指针
     */
    std::shared_ptr<StreamBase> getStream(const std::string &id);

    /**
     * @brief 获取所有推流对象
     *
     * @param streams 返回流对象map
     *
     * @return 0 成功，其它失败
     */
    int getAllStream(std::map<std::string, std::shared_ptr<StreamBase>> &streams);

protected:
    // 工作线程
    int svr();

private:
    std::map < std::string, std::shared_ptr<StreamBase>> m_streams;
    std::mutex m_streamLock;
    volatile bool m_stop;
    std::thread m_thread;
};


template <typename  T>
StreamMng<T>::StreamMng() : m_stop(true)
{

}

template <typename  T>
StreamMng<T>::~StreamMng()
{
    close();
}

template <typename  T>
int StreamMng<T>::open()
{
    m_stop = false;
    m_thread = std::thread(&StreamMng<T>::svr, this);
    return 0;
}

template <typename  T>
int StreamMng<T>::close()
{
    if (m_stop)
    {
        return 0;
    }
    m_stop = true;
    m_thread.join();
    m_streams.clear();
    return 0;
}

template <typename  T>
std::shared_ptr<StreamBase> StreamMng<T>::createStream(const std::string &id)
{
    StreamBase* s = dynamic_cast<StreamBase*>(new T());
    std::shared_ptr<StreamBase> sp = std::shared_ptr<StreamBase>(s);
    if (NULL != s)
    {
        std::lock_guard<std::mutex> guard(m_streamLock);
        m_streams[id] = sp;
    }
    return sp;
}

template <typename  T>
std::shared_ptr<StreamBase> StreamMng<T>::getStream(const std::string &id)
{
    std::lock_guard<std::mutex> guard(m_streamLock);
    std::map < std::string, std::shared_ptr<StreamBase>>::iterator it = m_streams.find(id);
    if (it != m_streams.end())
    {
        return it->second;
    }
    return std::shared_ptr<StreamBase>(NULL);
}

template <typename  T>
int StreamMng<T>::getAllStream(std::map<std::string, std::shared_ptr<StreamBase>> &streams)
{
    std::lock_guard<std::mutex> guard(m_streamLock);
    streams = m_streams;
    return 0;
}

template <typename  T>
int StreamMng<T>::svr()
{
    const uint64_t streamReleaseTime = 10;  // 已停止流回收时长

    while (!m_stop)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        uint64_t now = GetUtcCount();
        std::map < std::string, std::shared_ptr<StreamBase>>::iterator it;
        std::lock_guard<std::mutex> guard(m_streamLock);
        for (it = m_streams.begin(); it != m_streams.end(); )
        {
            const StreamInfo* fi = it->second->info();
            if (fi->status() == SS_STOPPED 
                && now > fi->stopTime()
                && now - fi->stopTime() > streamReleaseTime)
            {
                it->second->stop();
                it = m_streams.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    return 0;
}