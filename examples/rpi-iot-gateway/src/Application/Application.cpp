#include "Application.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>

#include "MeteoStationUart/MeteoStationUart.hpp"
#include "common/logs.h"

#ifndef APPL_VERSION
#define APPL_VERSION "dev"
#endif

static double getCpuTemp()
{
    std::ifstream file("/sys/class/thermal/thermal_zone0/temp");
    if (!file.is_open())
        return -1.0;

    int temp_milli;
    file >> temp_milli;
    return temp_milli / 1000.0;
}

Application::Application(const std::string &device_id, const std::string &device_secret,
                         const std::string &thing_id, int pub_period)
    : m_cloud(device_id, device_secret)
{
    std::cerr << std::endl;
    LOG_INF("Git version: " << APPL_VERSION);

    m_cloud.addThing(thing_id);
    m_cloud.setPeriod(pub_period);
}

Application::~Application() {}

PropertyUpdates Application::collectProperties()
{
    double cpu_temp  = -1;
    double home_temp = -1;
    double home_hum  = -1;

    if (!m_station->isAlive())
    {
        LOG_ERROR("Station watchdog timeout!");
    }
    else
    {
        home_temp = m_station->getTemperature();
        home_hum  = m_station->getHumidity();
    }

    cpu_temp = getCpuTemp();

    LOG_INF("CPU temperature " << cpu_temp << " C, Home temperature: " << home_temp
                               << " C, Home humidity = " << home_hum << " %");

    return {{"cpu_temperature", cpu_temp},
            {"home_temperature", home_temp},
            {"home_humidity", home_hum}};
}

void Application::stop()
{
    m_running = false;
    m_cv.notify_all();

    m_cloud.stop();
    LOG_INF("Exiting application...");
}

void Application::initStation()
{
    m_station = std::make_unique<MeteoStationUart>("/dev/ttyUSB0");

    if (!m_station->start())
    {
        LOG_ERROR("No serial devices found");
    }
}

void Application::start()
{
    initStation();
    m_cloud.start();

    std::unique_lock<std::mutex> lock(m_mutex);

    while (m_running)
    {
        lock.unlock();

        m_cloud.sync([this] { return collectProperties(); });

        lock.lock();
        m_cv.wait_for(lock, std::chrono::seconds(1), [this] { return !m_running.load(); });
    }

    m_cloud.stop();
    LOG_INF("Application stopped");
}
