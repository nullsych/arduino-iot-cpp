#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace mqtt
{
class async_client;
class ssl_options;
class connect_options;
}  // namespace mqtt

class MqttClient
{
public:
    MqttClient(const std::string &device_id, const std::string &device_secret);
    ~MqttClient();

    MqttClient(const MqttClient &)            = delete;
    MqttClient &operator=(const MqttClient &) = delete;

    void start();
    void stop();
    bool publish(const std::string &topic, const std::vector<uint8_t> &payload);

private:
    void initClient();
    bool ensureConnected();

private:
    std::string m_device_id;
    std::string m_device_secret;

    std::unique_ptr<mqtt::async_client> m_client;
    std::unique_ptr<mqtt::ssl_options> m_sslopts;
    std::unique_ptr<mqtt::connect_options> m_connopts;

    const int m_port = 8884;
    std::string m_url;
};
