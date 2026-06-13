#include "MqttClient.hpp"

#include <mqtt/async_client.h>

#include "logs.h"

MqttClient::MqttClient(const std::string &device_id, const std::string &device_secret)
    : m_device_id(device_id), m_device_secret(device_secret)
{
}

MqttClient::~MqttClient() = default;

void MqttClient::start()
{
    initClient();
    LOG_INF("Starting MQTT client at: " << m_url);
    ensureConnected();
}

void MqttClient::stop()
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

bool MqttClient::publish(const std::string &topic, const std::vector<uint8_t> &payload)
{
    if (!ensureConnected())
        return false;

    auto msg = mqtt::make_message(topic, payload.data(), payload.size());
    msg->set_qos(1);
    msg->set_retained(false);

    try
    {
        m_client->publish(msg)->wait();
        return true;
    }
    catch (const mqtt::exception &e)
    {
        LOG_ERROR("Failed to publish data: " << e.what());
        return false;
    }
}

void MqttClient::initClient()
{
    if (m_client)
        return;

    m_sslopts  = std::make_unique<mqtt::ssl_options>();
    m_connopts = std::make_unique<mqtt::connect_options>();

    m_connopts->set_user_name(m_device_id);
    m_connopts->set_password(m_device_secret);
    m_connopts->set_clean_session(true);
    m_connopts->set_automatic_reconnect(true);
    m_connopts->set_keep_alive_interval(30);
    m_connopts->set_ssl(*m_sslopts);

    m_url    = "ssl://iot.arduino.cc:" + std::to_string(m_port);
    m_client = std::make_unique<mqtt::async_client>(m_url, m_device_id);
}

bool MqttClient::ensureConnected()
{
    initClient();

    if (m_client->is_connected())
        return true;

    try
    {
        m_client->connect(*m_connopts)->wait();
        LOG_INF("MQTT connected");
        return true;
    }
    catch (const mqtt::exception &e)
    {
        LOG_ERROR("MQTT connecting error: " << e.what());
        return false;
    }
}
