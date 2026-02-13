#pragma once

#include <sstream>
#include <string>

#ifdef BUILD_DAEMON
#include <syslog.h>
#else
#include <ctime>
#include <iostream>
#endif

namespace logger
{

#ifndef BUILD_DAEMON

inline const char *RESET  = "\033[0m";
inline const char *RED    = "\033[31m";
inline const char *GREEN  = "\033[32m";
inline const char *YELLOW = "\033[33m";
inline const char *CYAN   = "\033[36m";

inline std::string timestamp()
{
    std::time_t now = std::time(nullptr);
    std::tm tm{};
    localtime_r(&now, &tm);

    char buffer[9];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);

    return buffer;
}

inline void log_console(const char *level, const char *color, const std::string &msg)
{
    std::cout << color << "[" << timestamp() << "] "
              << "(" << level << ") " << RESET << " " << msg << std::endl;
}

#else

inline void init_syslog(const char *name)
{
    openlog(name, LOG_PID | LOG_CONS, LOG_DAEMON);
}

inline void close_syslog()
{
    closelog();
}

inline void log_sys(int priority, const std::string &msg)
{
    syslog(priority, "%s", msg.c_str());
}

#endif

}  // namespace logger

#ifdef BUILD_DAEMON

#define LOG_STREAM(prio, msg)             \
    do                                    \
    {                                     \
        std::ostringstream oss;           \
        oss << msg;                       \
        logger::log_sys(prio, oss.str()); \
    } while (0)

#define LOG_INFO(msg) LOG_STREAM(LOG_INFO, msg)
#define LOG_WARN(msg) LOG_STREAM(LOG_WARNING, msg)
#define LOG_ERROR(msg) LOG_STREAM(LOG_ERR, msg)
#define LOG_DEBUG(msg) LOG_STREAM(LOG_DEBUG, msg)

#else

#define LOG_STREAM(level, color, msg)                 \
    do                                                \
    {                                                 \
        std::ostringstream oss;                       \
        oss << msg;                                   \
        logger::log_console(level, color, oss.str()); \
    } while (0)

#define LOG_INFO(msg) LOG_STREAM("INFO", logger::GREEN, msg)
#define LOG_WARN(msg) LOG_STREAM("WARN", logger::YELLOW, msg)
#define LOG_ERROR(msg) LOG_STREAM("ERRO", logger::RED, msg)
#define LOG_DEBUG(msg) LOG_STREAM("DBG", logger::CYAN, msg)

#endif

/* eof */