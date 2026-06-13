#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <utility>

#include "PropertyUpdate.hpp"

class Property
{
public:
    using Reader    = std::function<PropertyValue()>;
    using TimePoint = std::chrono::steady_clock::time_point;

    Property(std::string name, Reader reader) : m_name(std::move(name)), m_reader(std::move(reader))
    {
    }

    Property &publishEvery(int seconds)
    {
        m_period = std::chrono::seconds(seconds);
        return *this;
    }

    bool isDue(TimePoint now)
    {
        if (m_period.count() <= 0)
            return true;

        if (m_last_publish == TimePoint{} || now - m_last_publish >= m_period)
        {
            m_last_publish = now;
            return true;
        }

        return false;
    }

    PropertyUpdate read() const { return PropertyUpdate(m_name, m_reader()); }

private:
    std::string m_name;
    Reader m_reader;
    std::chrono::seconds m_period{0};
    TimePoint m_last_publish{};
};
