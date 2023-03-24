#include "tickcount.h"
#if defined(_WIN32) || defined(WIN32)  || defined(_WIN64) || defined(_WINDOWS)
#include <Windows.h>
#else 
#include <time.h>
#include <arpa/inet.h>
#endif
#include <chrono>

uint64_t GetMillisecondCounter()
{
#if defined(_WIN32) || defined(WIN32)  || defined(_WIN64) || defined(_WINDOWS)
	return (uint64_t)GetTickCount64();
#else 
	struct timespec tv = {};
	clock_gettime(CLOCK_MONOTONIC, &tv);
	return ((uint64_t)tv.tv_sec * (uint64_t)1000) + ((uint64_t)tv.tv_nsec / 1000000);
#endif
}

uint64_t GetSecondCounter()
{
#if defined(_WIN32) || defined(WIN32)  || defined(_WIN64) || defined(_WINDOWS)
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