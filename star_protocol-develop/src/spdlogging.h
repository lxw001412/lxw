/**
 * @file spdlogging.h
 * @brief  spdlog library wrapper
 * @author Bill
 * @version 0.0.1
 * @date 2021-06-15
 */

#pragma once

#ifdef _DEBUG
    #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#else
    #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#endif  // DEBUG

#include <spdlog/spdlog.h>
#include <string>

namespace star_protocol
{

struct logParam
{
    std::string fileLogLevel;       // 文件日志级别: trace,debug,warn,error,critical
    std::string fileLogPath;        // 日志文件路径
    int fileLogNum;                 // 日志文件个数
    int fileLogSize;                // 日志文件大小，单位MB
    bool enableConsoleLog;          // 是否在控制台显示
    std::string consoleLogLevel;    // 控制台日志级别: trace,debug,warn,error,critical
};

extern int initSpdLog(const logParam &param);
extern void finiSpdLog();

}

#define SPDTRACE(...)    SPDLOG_TRACE(__VA_ARGS__)
#define SPDDEBUG(...)    SPDLOG_DEBUG(__VA_ARGS__)
#define SPDINFO(...)     SPDLOG_INFO(__VA_ARGS__)
#define SPDWARN(...)     SPDLOG_WARN(__VA_ARGS__)
#define SPDERROR(...)    SPDLOG_ERROR(__VA_ARGS__)
#define SPDCRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)