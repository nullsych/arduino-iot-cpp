#include "CborPacker.hpp"

#include <cbor.h>

#include <cstdint>
#include <cstdlib>
#include <string>
#include <variant>
#include <vector>

std::vector<uint8_t> CborPacker::make(const std::string &name, double value)
{
    cbor_item_t *map = new_map_with_name_(name, /*pairs=*/2);
    add_value_(map, value);
    return pack_one_record_(map);
}

std::vector<uint8_t> CborPacker::make(const std::string &name, uint32_t value)
{
    cbor_item_t *map = new_map_with_name_(name, 2);
    add_value_(map, value);
    return pack_one_record_(map);
}

std::vector<uint8_t> CborPacker::make(const std::string &name, bool value)
{
    cbor_item_t *map = new_map_with_name_(name, 2);
    add_value_(map, value);
    return pack_one_record_(map);
}

std::vector<uint8_t> CborPacker::make(const std::string &name, const std::string &value)
{
    cbor_item_t *map = new_map_with_name_(name, 2);
    add_value_(map, value);
    return pack_one_record_(map);
}

std::vector<uint8_t> CborPacker::make_multiple(const PropertyUpdates &items)
{
    cbor_item_t *arr = cbor_new_definite_array(items.size());

    for (const auto &item : items)
    {
        cbor_item_t *map = new_map_with_name_(item.name, 2);
        std::visit([map](const auto &value) { add_value_(map, value); }, item.value);

        cbor_array_push(arr, map);
        cbor_decref(&map);
    }

    unsigned char *buffer = nullptr;
    size_t buffer_size    = 0;
    size_t len            = cbor_serialize_alloc(arr, &buffer, &buffer_size);

    std::vector<uint8_t> out(buffer, buffer + len);

    free(buffer);
    cbor_decref(&arr);

    return out;
}

std::vector<uint8_t> CborPacker::make(const std::string &name, const char *value)
{
    return make(name, std::string(value ? value : ""));
}

cbor_item_t *CborPacker::new_map_with_name_(const std::string &name, size_t pairs)
{
    cbor_item_t *map = cbor_new_definite_map(pairs);

    (void)cbor_map_add(map, (cbor_pair){
                                .key   = cbor_build_uint8(0),
                                .value = cbor_build_string(name.c_str()),
                            });

    return map;
}

void CborPacker::add_value_(cbor_item_t *map, double value)
{
    (void)cbor_map_add(map, (cbor_pair){
                                .key   = cbor_build_uint8(2),
                                .value = cbor_build_float8(value),
                            });
}

void CborPacker::add_value_(cbor_item_t *map, uint32_t value)
{
    (void)cbor_map_add(map, (cbor_pair){
                                .key   = cbor_build_uint8(2),
                                .value = cbor_build_uint32(value),
                            });
}

void CborPacker::add_value_(cbor_item_t *map, bool value)
{
    (void)cbor_map_add(map, (cbor_pair){
                                .key   = cbor_build_uint8(4),
                                .value = cbor_build_bool(value),
                            });
}

void CborPacker::add_value_(cbor_item_t *map, const std::string &value)
{
    (void)cbor_map_add(map, (cbor_pair){
                                .key   = cbor_build_uint8(3),
                                .value = cbor_build_string(value.c_str()),
                            });
}

std::vector<uint8_t> CborPacker::pack_one_record_(cbor_item_t *map)
{
    unsigned char *buffer = nullptr;
    size_t buffer_size    = 0;
    size_t len            = 0;

    cbor_item_t *arr      = cbor_new_definite_array(1);
    cbor_array_push(arr, map);

    len = cbor_serialize_alloc(arr, &buffer, &buffer_size);

    std::vector<uint8_t> out(buffer, buffer + len);

    free(buffer);
    cbor_decref(&arr);
    return out;
}
