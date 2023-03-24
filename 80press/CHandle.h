#pragma once
#include "ace/Event_Handler.h"
#include "ace/INET_Addr.h"
#include "json/json.h"
#include "ace/SOCK_Dgram.h"
class IHandler
{
public:
	virtual int handle(const Json::Value &data, const ACE_INET_Addr &from_addr) = 0;
	virtual int handle(const char* buf, const ACE_INET_Addr &from_addr, size_t size) = 0;
};

class INetwork
{
public:
	virtual int sendto(const Json::Value& data, const ACE_INET_Addr &to_addr) = 0;
	virtual int sendto(const char* buf, const ACE_INET_Addr &to_addr, size_t size) = 0;
	virtual void addHandler(IHandler *handler) = 0;
	virtual void rmHandler(IHandler *handler) = 0;
	virtual long add_timer_handler(ACE_Event_Handler* handler, const void* arg, int delay, int interval) = 0;
	virtual void rm_timer_handler(/*long timerId*/ACE_Event_Handler * handler) = 0;
};