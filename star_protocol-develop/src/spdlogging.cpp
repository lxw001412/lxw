#include "spdlogging.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h> 
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

namespace star_protocol
{

#ifdef _DEBUG
#define SPDLOG_PATTERN     "[%Y-%m-%d %H:%M:%S.%e][%^%L%$][%t] %v [%@]"
#else
#define SPDLOG_PATTERN     "[%Y-%m-%d %H:%M:%S.%e][%^%L%$][%t] %v"
#endif
    
spdlog::level::level_enum convertLogLevel(const std::string &level);

int initSpdLog(const logParam &param)
{
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        param.fileLogPath, 
        1024 * 1024 * param.fileLogSize, 
        param.fileLogNum);
    file_sink->set_level(convertLogLevel(param.fileLogLevel));
    file_sink->set_pattern(SPDLOG_PATTERN);

    if (param.enableConsoleLog)
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(convertLogLevel(param.consoleLogLevel));
        console_sink->set_pattern(SPDLOG_PATTERN);

        spdlog::init_thread_pool(8192, 1);
        auto logger = std::make_shared<spdlog::async_logger>("spdlog", 
            spdlog::sinks_init_list({file_sink, console_sink}), 
            spdlog::thread_pool(), 
            spdlog::async_overflow_policy::block);
        spdlog::set_default_logger(logger);
    }
    else
    {
        spdlog::init_thread_pool(8192, 1);
        auto logger = std::make_shared<spdlog::async_logger>("spdlog", 
            spdlog::sinks_init_list({file_sink}), 
            spdlog::thread_pool(), 
            spdlog::async_overflow_policy::block);
        spdlog::set_default_logger(logger);
    }
    spdlog::set_level(std::min(convertLogLevel(param.fileLogLevel), convertLogLevel(param.consoleLogLevel)));
    return 0;
}

void finiSpdLog()
{
    spdlog::shutdown();
}

spdlog::level::level_enum convertLogLevel(const std::string &level)
{
    if (level == "trace")
    {
        return spdlog::level::trace;
    }
    else if (level == "debug")
    {
        return spdlog::level::debug;
    }
    else if (level == "info")
    {
        return spdlog::level::info;
    }
    else if (level == "warn")
    {
        return spdlog::level::warn;
    }
    else if (level == "error")
    {
        return spdlog::level::err;
    }
    else if (level == "critical")
    {
        return spdlog::level::critical;
    }
    else
    {
        return spdlog::level::info;
    }
}

}