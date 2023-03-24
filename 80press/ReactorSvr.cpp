#include "ReactorSvr.h"
#include <ace/Sched_Params.h>
#include <ace/OS.h>
#include <stdio.h>
#include "CLog.h"
#ifdef _WIN32
#include <mmSystem.h>
#pragma comment(lib, "winmm.lib")
#endif // _WIN32

int ReactorSvr::start(bool setMaxThreadPriority)
{
	m_setMaxThreadPriority = setMaxThreadPriority;
	std::thread* thread = nullptr;
	for (int i = 0; i < 20; i++)
	{
		thread = new std::thread(&ReactorSvr::loop, this);
		m_threads.push_back(thread);
	}

	return 0;
}

int ReactorSvr::stop(void)
{
	if (m_run)
	{
		m_run = false;
	//	m_reactor.end_reactor_event_loop();
		/* if (m_thread.joinable())
		 {
			 m_thread.join();
		 }*/
		for (auto thread : m_threads)
		{
			if (thread)
			{
				if (thread->joinable())
				{
					thread->join();
					delete thread;
					thread = nullptr;
				}
			}
		}
	}
	return 0;
}

bool ReactorSvr::register_handler(ACE_Event_Handler* handler)
{
	if (m_reactor.register_handler(handler, ACE_Event_Handler::READ_MASK) != 0)
	{
		return false;
	}
	return 0;
}

void ReactorSvr::remove_handler(ACE_Event_Handler* handler)
{
	m_reactor.remove_handler(handler, ACE_Event_Handler::READ_MASK | ACE_Event_Handler::DONT_CALL);
}

long ReactorSvr::register_timer_handler(ACE_Event_Handler * handler, const void * arg, int delay, int interval)
{
	ACE_Time_Value tDelay(delay / 1000, (delay % 1000) * 1000);
	ACE_Time_Value tInterval(interval / 1000, (interval % 1000) * 1000);
	return  m_reactor.schedule_timer(handler, arg, tDelay, tInterval);
}

void ReactorSvr::remove_timer_handler(ACE_Event_Handler* handler)
{
	if (handler->reactor() != nullptr)
	{
		handler->reactor()->cancel_timer(handler);
	}
}

int ReactorSvr::loop(void)
{
	if (m_setMaxThreadPriority)
	{
		set_thread_max_priority();
	}

	// 设置设置精度为1毫秒
#ifdef _WIN32
	if (timeBeginPeriod(1) != TIMERR_NOERROR)
	{
		LOG_E("定时器精度设置出错");
	}
#endif // _WIN32
	while (m_run)
	{
		try
		{
			m_reactor.owner(ACE_Thread::self());
			/*m_reactor.run_reactor_event_loop();*/
			m_reactor.handle_events();
		}
		catch (...)
		{
			;
		}
	}

#ifdef _WIN32
	timeEndPeriod(1);
#endif // _WIN32
	return 0;
}

void ReactorSvr::set_thread_max_priority()
{
	int priority;
	int policy;
	const int updatePolicy = ACE_SCHED_FIFO;
	// ACE_SCHED_FIFO: 实时调度策略，先到先服务; 
	// ACE_SCHED_RR: 实时调度策略，先到先服务; 
	// SCHED_OTHER: 分时调度策略 

	ACE_hthread_t tid;
	ACE_OS::thr_self(tid);

	ACE_OS::thr_getprio(tid, priority, policy);
	printf("default: priority: %d, policy: %d\n", priority, policy);

	priority = ACE_Sched_Params::priority_max(ACE_SCHED_RR);

	printf("max priority: %d\n", priority);
	ACE_OS::thr_setprio(tid, priority, ACE_SCHED_RR);

	ACE_OS::thr_getprio(tid, priority, policy);
	printf("update: priority: %d, policy: %d\n", priority, policy);
}

static ACE_THR_FUNC_RETURN event_loop(void *arg)
{
	ACE_Reactor *reactor = static_cast<ACE_Reactor *> (arg);
	reactor->owner(ACE_OS::thr_self());
	reactor->run_reactor_event_loop();
	return 0;
}
