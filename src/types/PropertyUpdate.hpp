#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <variant>
#include <vector>

using PropertyValue = std::variant<double, uint32_t, bool, std::string>;

struct PropertyUpdate
{
    std::string name;
    PropertyValue value;

    PropertyUpdate(std::string property_name, double property_value)
        : name(std::move(property_name)), value(property_value)
    {
    }

    PropertyUpdate(std::string property_name, float property_value)
        : name(std::move(property_name)), value(static_cast<double>(property_value))
    {
    }

    PropertyUpdate(std::string property_name, int property_value)
        : name(std::move(property_name)),
          value(property_value >= 0 ? PropertyValue(static_cast<uint32_t>(property_value))
                                    : PropertyValue(static_cast<double>(property_value)))
    {
    }

    PropertyUpdate(std::string property_name, uint32_t property_value)
        : name(std::move(property_name)), value(property_value)
    {
    }

    PropertyUpdate(std::string property_name, PropertyValue property_value)
        : name(std::move(property_name)), value(std::move(property_value))
    {
    }

    PropertyUpdate(std::string property_name, bool property_value)
        : name(std::move(property_name)), value(property_value)
    {
    }

    PropertyUpdate(std::string property_name, std::string property_value)
        : name(std::move(property_name)), value(std::move(property_value))
    {
    }

    PropertyUpdate(std::string property_name, const char *property_value)
        : name(std::move(property_name)), value(std::string(property_value ? property_value : ""))
    {
    }
};

using PropertyUpdates = std::vector<PropertyUpdate>;
