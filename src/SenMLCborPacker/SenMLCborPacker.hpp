#pragma once

#include <cbor.h>

#include <cstdint>
#include <string>
#include <vector>

class SenMLCborPacker
{
public:
    static std::vector<uint8_t> make(const std::string &name, double value);
    static std::vector<uint8_t> make(const std::string &name, uint32_t value);
    static std::vector<uint8_t> make(const std::string &name, bool value);
    static std::vector<uint8_t> make(const std::string &name, const std::string &value);
    static std::vector<uint8_t> make(const std::string &name, const char *value);

private:
    static cbor_item_t *new_map_with_name_(const std::string &name, size_t pairs);
    static std::vector<uint8_t> pack_one_record_(cbor_item_t *map);
};
