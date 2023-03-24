#pragma once
#include <deque>
#include <mutex>
#include <condition_variable>

template <typename  T, typename Container = std::deque<T> >
class MessageQueue
{
public:
    MessageQueue(void);
    ~ MessageQueue(void);

    int start(void);

    int stop(void);

    int push(const T & val);

    //无数据会阻塞，直到调用stop
    int pop(T & val);

    /*
        参数
        ms: 超时时间,毫秒
    */
    int pop(T & val, unsigned int ms);

    int size(void);

protected:
    bool m_run = true;
    Container m_msg;
    std::mutex m_msg_mux;
    std::condition_variable m_condVar;
};

template <typename  T,  typename Container /*= std::deque<T> */>
MessageQueue<T, Container>::MessageQueue(void)
{
}

template <typename  T, typename Container /*= std::deque<T> */>
MessageQueue<T, Container>::~MessageQueue(void)
{
}

template <typename  T, typename Container /*= std::deque<T> */>
int MessageQueue<T, Container>::start(void)
{
    std::lock_guard<std::mutex> guard(m_msg_mux);
    m_msg.clear();
    m_run = true;
    return 0;
}

template <typename  T, typename Container /*= std::deque<T> */>
int MessageQueue<T, Container>::stop(void)
{
    do 
    {
        std::lock_guard<std::mutex> guard(m_msg_mux);
        m_run = false;
        m_msg.clear();
    } while (0);
    m_condVar.notify_all();
    return 0;
}

template <typename  T, typename Container /*= std::deque<T> */ >
int MessageQueue<T, Container>::size(void)
{
    std::lock_guard<std::mutex> guard(m_msg_mux);
    return (int)m_msg.size();
}

template <typename  T, typename Container /*= std::deque<T> */>
int MessageQueue<T, Container>::pop(T & val)
{    
    std::unique_lock<std::mutex> lock(m_msg_mux);
    m_condVar.wait(lock, [this] 
    {
        if ((!m_run) || m_msg.size() != 0)
        {
            return true;
        }                    
        return false;
    });

    if (!m_msg.empty())
    {
        val = m_msg.front();
        m_msg.pop_front();
        return 0;
    }
    return -1;
}

template <typename  T, typename Container /*= std::deque<T> */>
int MessageQueue<T, Container>::pop(T & val, unsigned int ms)
{
    std::unique_lock<std::mutex> lock(m_msg_mux);
    m_condVar.wait_for(lock, std::chrono::milliseconds(ms), [this]
    {
        if ((!m_run) || m_msg.size() != 0)
        {
            return true;
        }
        return false;
    });

    if (!m_msg.empty())
    {
        val = m_msg.front();
        m_msg.pop_front();
        return 0;
    }

    return -1;
}


template <typename T, typename Container /*= std::deque<T> */>
int MessageQueue<T, Container>::push(const T & val)
{
    do 
    {
        std::lock_guard<std::mutex> guard(m_msg_mux);
        m_msg.push_back(val);
    } while (0);
    m_condVar.notify_one();
    return 0;
}




