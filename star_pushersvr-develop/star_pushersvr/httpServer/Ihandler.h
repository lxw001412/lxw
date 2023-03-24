#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif // _WIN32

#include "httplib.h"

using namespace httplib;

class IHandler
{
public:
	virtual void get(const Request &, Response &) {};
	virtual void post(const Request &, Response &){};
	virtual void put(const Request &, Response &) {};
	virtual void del(const Request &, Response &) {};
};

