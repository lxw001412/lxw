#pragma once

#include <stdint.h>

uint64_t GetMillisecondCounter();
uint64_t GetSecondCounter();

/*
 *  取UTC毫秒值
*/
uint64_t GetUtcMsCount();