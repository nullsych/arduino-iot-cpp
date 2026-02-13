#pragma once

#include <mqtt/async_client.h>

#include <memory>
#include <string>
#include <vector>

class Application
{
private:
    /* data */
public:
    Application(const std::string &, const std::string &);
    ~Application();

public:
    void start();
    void addThing(const std::string &thing_id);

private:
    void broadcast();

private:
    std::string m_device_id     = "";
    std::string m_device_secret = "";

private:
    std::unique_ptr<mqtt::async_client> m_client;
    std::unique_ptr<mqtt::ssl_options> m_sslopts;
    std::unique_ptr<mqtt::connect_options> m_connopts;
    std::vector<std::string> m_topics_out;

private: /* url members */
    const int m_port1 = 8884;
    const int m_port2 = 8885;
    std::string m_url = "ssl://iot.arduino.cc:";
};
