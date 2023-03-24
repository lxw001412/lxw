#pragma once
#include <list>
#include <string>
#include <atomic>
#include <future>
#include <thread>
#include <ace/Mutex.h>
#include <ace/Singleton.h>
#include <ace/Reactor.h>
#include <vector>
#include <ace/WFMO_Reactor.h>
class ReactorSvr
{
public:
    ReactorSvr() {}
	virtual ~ReactorSvr() {}

    int start(bool setMaxThreadPriority);

    int stop(void);

    // 注册IO处理器
    bool register_handler(ACE_Event_Handler* handler);

    // 移除IO处理器
    void remove_handler(ACE_Event_Handler* handler);

    //注册定时器
    // delay   ----- 单位 ms
    long register_timer_handler(ACE_Event_Handler * handler, const void * arg, int delay, int interval);

    //移除定时器
    void remove_timer_handler(ACE_Event_Handler* handler);

protected:
    int loop(void);

    void set_thread_max_priority();

protected:
    //IO反应器
	ACE_WFMO_Reactor m_reactor;
    volatile bool m_run = true;
    std::vector<std::thread*> m_threads;
    volatile bool m_setMaxThreadPriority = false;
};
