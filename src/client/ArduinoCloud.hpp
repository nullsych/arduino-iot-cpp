#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "PropertyUpdate.hpp"
#include "Thing.hpp"

class MqttClient;

class ArduinoCloud
{
public:
    ArduinoCloud(const std::string &device_id, const std::string &device_secret);
    ~ArduinoCloud();

    using PropertyReader = std::function<PropertyUpdates()>;

    void start();
    void stop();
    void addThing(const std::string &thing_id);
    void setPeriod(int seconds);
    void sync(const PropertyReader &reader);
    void publish(const PropertyUpdates &updates);

private:
    bool isPublishDue(std::chrono::steady_clock::time_point now) const;

private:
    std::vector<Thing> m_things;
    std::unique_ptr<MqttClient> m_mqtt;

    std::atomic<uint64_t> m_sent_count{0};
    std::atomic<uint64_t> m_failed_count{0};

    std::chrono::seconds m_publish_interval{60};
    std::chrono::steady_clock::time_point m_last_publish_attempt{};
    bool m_has_publish_attempt = false;
};
