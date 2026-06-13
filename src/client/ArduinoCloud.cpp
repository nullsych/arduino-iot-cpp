#include "ArduinoCloud.hpp"

#include "CborPacker.hpp"
#include "MqttClient.hpp"
#include "logs.h"

ArduinoCloud::ArduinoCloud(const std::string &device_id, const std::string &device_secret)
    : m_mqtt(std::make_unique<MqttClient>(device_id, device_secret))
{
}

ArduinoCloud::~ArduinoCloud() = default;

void ArduinoCloud::addThing(const std::string &thing_id)
{
    m_things.emplace_back(thing_id);
}

void ArduinoCloud::setPeriod(int seconds)
{
    m_publish_interval = std::chrono::seconds(seconds);
}

void ArduinoCloud::sync(const PropertyReader &reader)
{
    if (!reader)
        return;

    const auto now = std::chrono::steady_clock::now();

    if (!isPublishDue(now))
        return;

    m_last_publish_attempt = now;
    m_has_publish_attempt  = true;

    publish(reader());
}

void ArduinoCloud::start()
{
    m_mqtt->start();
}

void ArduinoCloud::stop()
{
    m_mqtt->stop();
    LOG_INF("ArduinoCloud stopped");
}

void ArduinoCloud::publish(const PropertyUpdates &updates)
{
    if (updates.empty())
        return;

    if (m_things.empty())
    {
        LOG_WARN("No Arduino IoT Things registered");
        return;
    }

    auto payload = CborPacker::make_multiple(updates);

    for (const auto &thing : m_things)
    {
        if (m_mqtt->publish(thing.outTopic(), payload))
            m_sent_count++;
        else
            m_failed_count++;
    }

    uint64_t sent   = m_sent_count.load();
    uint64_t failed = m_failed_count.load();
    LOG_INF("Published " << sent << " packet(s) / Failed " << failed << " packet(s)");
}

bool ArduinoCloud::isPublishDue(std::chrono::steady_clock::time_point now) const
{
    if (!m_has_publish_attempt)
        return true;

    return now - m_last_publish_attempt >= m_publish_interval;
}
