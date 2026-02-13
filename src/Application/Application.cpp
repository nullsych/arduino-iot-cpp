#include "Application.hpp"

#include <cbor.h>
#include <mqtt/async_client.h>

#include <memory>

#include "SenMLCborPacker/SenMLCborPacker.hpp"
#include "common/logs.h"

#ifndef APPL_VERSION
#define APPL_VERSION "dev"
#endif

Application::Application(const std::string &device_id, const std::string &device_secret)
    : m_device_id(device_id), m_device_secret(device_secret)
{
    LOG_INFO("Git version: " << "dev");
}

Application::~Application() {}

void Application::addThing(const std::string &thing_id)
{
    std::string topic = "/a/t/" + thing_id + "/e/o";
    m_topics_out.push_back(topic);
}

void Application::broadcast()
{
    double cpu_temp = 50.666;
    auto payload    = SenMLCborPacker::make("cpu_temperature", cpu_temp);

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

    try
    {
        m_client->connect(*m_connopts)->wait();
        LOG_INFO("MQTT connected");

        LOG_INFO("Notify all topics");
        broadcast();

        m_client->disconnect()->wait();
    }
    catch (const mqtt::exception &e)
    {
        LOG_INFO("MQTT error: " << e.what());
    }
}