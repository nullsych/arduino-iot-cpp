#pragma once

#include <sstream>
#include <string>

#ifdef BUILD_DAEMON
#include <syslog.h>
#else
#include <ctime>
#include <iostream>
#endif

namespace arduino_iot_cloud_logger
{

#ifndef BUILD_DAEMON

inline const char *RESET  = "\033[0m";
inline const char *RED    = "\033[31m";
inline const char *GREEN  = "\033[32m";
inline const char *YELLOW = "\033[33m";

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

inline void log_sys(int priority, const std::string &msg)
{
    syslog(priority, "%s", msg.c_str());
}

#endif

}  // namespace arduino_iot_cloud_logger

#ifdef BUILD_DAEMON

#define LOG_INF(msg)                                            \
    do                                                          \
    {                                                           \
        std::ostringstream oss;                                 \
        oss << msg;                                             \
        arduino_iot_cloud_logger::log_sys(LOG_INFO, oss.str()); \
    } while (0)
#define LOG_WARN(msg)                                              \
    do                                                             \
    {                                                              \
        std::ostringstream oss;                                    \
        oss << msg;                                                \
        arduino_iot_cloud_logger::log_sys(LOG_WARNING, oss.str()); \
    } while (0)
#define LOG_ERROR(msg)                                         \
    do                                                         \
    {                                                          \
        std::ostringstream oss;                                \
        oss << msg;                                            \
        arduino_iot_cloud_logger::log_sys(LOG_ERR, oss.str()); \
    } while (0)

#else

#define LOG_INF(msg)                                                                               \
    do                                                                                             \
    {                                                                                              \
        std::ostringstream oss;                                                                    \
        oss << msg;                                                                                \
        arduino_iot_cloud_logger::log_console("INFO", arduino_iot_cloud_logger::GREEN, oss.str()); \
    } while (0)
#define LOG_WARN(msg)                                                                   \
    do                                                                                  \
    {                                                                                   \
        std::ostringstream oss;                                                         \
        oss << msg;                                                                     \
        arduino_iot_cloud_logger::log_console("WARN", arduino_iot_cloud_logger::YELLOW, \
                                              oss.str());                               \
    } while (0)
#define LOG_ERROR(msg)                                                                           \
    do                                                                                           \
    {                                                                                            \
        std::ostringstream oss;                                                                  \
        oss << msg;                                                                              \
        arduino_iot_cloud_logger::log_console("ERRO", arduino_iot_cloud_logger::RED, oss.str()); \
    } while (0)

#endif
