#pragma once

#include <cbor.h>

#include <cstdint>
#include <string>
#include <vector>

#include "PropertyUpdate.hpp"

class CborPacker
{
public:
    static std::vector<uint8_t> make(const std::string &name, double value);
    static std::vector<uint8_t> make(const std::string &name, uint32_t value);
    static std::vector<uint8_t> make(const std::string &name, bool value);
    static std::vector<uint8_t> make(const std::string &name, const std::string &value);
    static std::vector<uint8_t> make(const std::string &name, const char *value);
    static std::vector<uint8_t> make_multiple(const PropertyUpdates &items);

private:
    static cbor_item_t *new_map_with_name_(const std::string &name, size_t pairs);
    static void add_value_(cbor_item_t *map, double value);
    static void add_value_(cbor_item_t *map, uint32_t value);
    static void add_value_(cbor_item_t *map, bool value);
    static void add_value_(cbor_item_t *map, const std::string &value);
    static std::vector<uint8_t> pack_one_record_(cbor_item_t *map);
};
