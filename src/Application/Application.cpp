#include "Application.hpp"

#include <cbor.h>
#include <mqtt/async_client.h>

#include <fstream>
#include <iostream>
#include <memory>

#include "SenMLCborPacker/SenMLCborPacker.hpp"
#include "StationData/StationData.hpp"
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

Application::Application(const std::string &device_id, const std::string &device_secret)
    : m_device_id(device_id), m_device_secret(device_secret)
{
    std::cout << std::endl;
    LOG_INF("Git version: " << APPL_VERSION);
}

Application::~Application() {}

void Application::addThing(const std::string &thing_id)
{
    std::string topic = "/a/t/" + thing_id + "/e/o";
    m_topics_out.push_back(topic);
}

/**
 * @brief Set publishing maximum period.
 * @param p period is seconds.
 */
void Application::setPeriod(int p)
{
    if (p < 30 || p <= 0)
    {
        LOG_INF("Period " << p << " is too small! Recommend to set at least 30 seconds!");
    }
    else
    {
        m_pub_interval = p;
        LOG_INF("Set publish period: " << m_pub_interval << " sec");
    }
}

void Application::broadcast()
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

    auto payload = SenMLCborPacker::make_multiple({{"cpu_temperature", cpu_temp},
                                                   {"home_temperature", home_temp},
                                                   {"home_humidity", home_hum}});
    for (const auto &topic : m_topics_out)
    {
        auto msg = mqtt::make_message(topic, payload.data(), payload.size());
        msg->set_qos(1);
        msg->set_retained(false);

        try
        {
            m_client->publish(msg)->wait();
            m_sent_count++;
        }
        catch (const mqtt::exception &e)
        {
            m_failed_count++;
            LOG_ERROR("Failed to publish data: " << e.what());
        }

        uint64_t sent   = m_sent_count.load();
        uint64_t failed = m_failed_count.load();
        LOG_INF("Published " << sent << " packet(s) / Failed " << failed << " packet(s)");
    }
}

void Application::stopClient()
{
    try
    {
        if (m_client && m_client->is_connected())
            m_client->disconnect()->wait();
    }
    catch (...)
    {
    }
}

void Application::stop()
{
    m_running = false;
    m_cv.notify_all();

    stopClient();
    LOG_INF("Exiting application...");
}

void Application::initStation()
{
    m_station = std::make_unique<StationData>("/dev/ttyUSB0");

    if (!m_station->start())
    {
        LOG_ERROR("No serial devices found");
    }
}

void Application::initClient()
{
    m_sslopts  = std::make_unique<mqtt::ssl_options>();
    m_connopts = std::make_unique<mqtt::connect_options>();

    m_connopts->set_user_name(m_device_id);
    m_connopts->set_password(m_device_secret);
    m_connopts->set_clean_session(true);
    m_connopts->set_automatic_reconnect(true);
    m_connopts->set_keep_alive_interval(30);
    m_connopts->set_ssl(*m_sslopts);

    m_url    = m_url + std::to_string(m_port1);
    m_client = std::make_unique<mqtt::async_client>(m_url, m_device_id);
}

void Application::start()
{
    initStation();
    initClient();

    LOG_INF("Starting MQTT client at: " << m_url);

    bool cold_start = true;
    std::unique_lock<std::mutex> lock(m_mutex);

    while (m_running)
    {
        lock.unlock();

        try
        {
            if (!m_client->is_connected() && cold_start)
            {
                cold_start = false;
                m_client->connect(*m_connopts)->wait();
                LOG_INF("MQTT connected");
            }
            /* reconnection done automatically with exponential delay on Paho side */

            /* publish data anyway to trigger all statistic */
            broadcast();
        }
        catch (const mqtt::exception &e)
        {
            LOG_ERROR("MQTT connecting error: " << e.what());
        }

        lock.lock();
        /* SMALL DELAY */
        m_cv.wait_for(lock, std::chrono::seconds(m_pub_interval),
                      [this] { return !m_running.load(); });
    }

    stopClient();
    LOG_INF("Application stopped");
}
