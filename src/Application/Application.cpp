#include "Application.hpp"

#include <cbor.h>
#include <mqtt/async_client.h>

#include <fstream>
#include <iostream>
#include <memory>

#include "SenMLCborPacker/SenMLCborPacker.hpp"
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
    LOG_INFO("Git version: " << APPL_VERSION);
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
        LOG_INFO("Period " << p << " is too small! Recommend to set at least 30 seconds!");
    }
    else
    {
        m_pub_interval = p;
    }
}

void Application::broadcast()
{
    double cpu_temp = getCpuTemp();
    LOG_INFO("Read RPi CPU temperature: " << cpu_temp << " C");

    auto payload = SenMLCborPacker::make("cpu_temperature", cpu_temp);

#if 0
            auto payload = make_senml_pack_double({
            {"cpu_temperature", cpu_temp},
            {"home_temperature", home_temp},
            {"home_humidity", home_hum}
            });
#endif

    for (const auto &topic : m_topics_out)
    {
        auto msg = mqtt::make_message(topic, payload.data(), payload.size());
        msg->set_qos(1);
        msg->set_retained(false);

        m_client->publish(msg)->wait();
        LOG_INFO("Published " << payload.size() << " bytes");
    }
}

void Application::stop()
{
    m_running = false;

    if (m_client && m_client->is_connected())
        m_client->disconnect()->wait();
}

void Application::start()
{
    m_sslopts  = std::make_unique<mqtt::ssl_options>();
    m_connopts = std::make_unique<mqtt::connect_options>();

    m_connopts->set_user_name(m_device_id);
    m_connopts->set_password(m_device_secret);
    m_connopts->set_clean_session(true);
    m_connopts->set_keep_alive_interval(30);
    m_connopts->set_ssl(*m_sslopts);

    m_url    = m_url + std::to_string(m_port1);
    m_client = std::make_unique<mqtt::async_client>(m_url, m_device_id);

    LOG_INFO("Starting client at: " << m_url);

    try
    {
        m_client->connect(*m_connopts)->wait();
        LOG_INFO("MQTT connected");

        while (m_running)
        {
            LOG_INFO("Broadcast data...");
            broadcast();

            std::this_thread::sleep_for(std::chrono::seconds(m_pub_interval));
        }

        m_client->disconnect()->wait();
    }
    catch (const mqtt::exception &e)
    {
        LOG_INFO("MQTT connecting error: " << e.what());
    }
}