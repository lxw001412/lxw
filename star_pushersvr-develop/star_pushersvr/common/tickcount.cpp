#include "tickcount.h"
#if defined WIN32 || defined WIN64
#include <Windows.h>
#else 
#include <time.h>
#include <arpa/inet.h>
#endif
#include <chrono>

uint64_t GetMillisecondCounter()
{
#ifdef WIN32
    LARGE_INTEGER freq;
    LARGE_INTEGER count;
    if (QueryPerformanceFrequency(&freq)
        && QueryPerformanceCounter(&count))
    {
        return (uint64_t)(count.QuadPart * 1000 / freq.QuadPart);
    }
    return (uint64_t)GetTickCount64();
#else 
	struct timespec tv = {};
	clock_gettime(CLOCK_MONOTONIC, &tv);
	return ((uint64_t)tv.tv_sec * (uint64_t)1000) + ((uint64_t)tv.tv_nsec / 1000000);
#endif
}

uint64_t GetSecondCounter()
{
#ifdef WIN32
    LARGE_INTEGER freq;
    LARGE_INTEGER count;
    if (QueryPerformanceFrequency(&freq)
        && QueryPerformanceCounter(&count))
    {
        return (uint64_t)(count.QuadPart / freq.QuadPart);
    }
    return (uint64_t)GetTickCount64() / 1000;
#else 
	struct timespec tv = {};
	clock_gettime(CLOCK_MONOTONIC, &tv);
	return (uint64_t)tv.tv_sec;
#endif
}

uint64_t GetUtcMsCount()
{
  return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch()).count();
}

uint64_t GetUtcCount()
{
    return (uint64_t)std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}