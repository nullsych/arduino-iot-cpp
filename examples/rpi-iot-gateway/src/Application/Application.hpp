#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>

#include "ArduinoCloud.hpp"
#include "MeteoStationUart.hpp"

class Application
{
private:
    /* data */
public:
    Application(const std::string &device_id, const std::string &device_secret,
                const std::string &thing_id, int pub_period);
    ~Application();

public:
    void start();
    void stop();

private:
    PropertyUpdates collectProperties();
    void initStation();

private:
    ArduinoCloud m_cloud;

private: /* client control  */
    std::atomic<bool> m_running{true};
    std::condition_variable m_cv;
    std::mutex m_mutex;

private:
    std::unique_ptr<MeteoStationUart> m_station;
};
