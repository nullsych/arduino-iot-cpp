#include "SenMLCborPacker.hpp"

#include <cbor.h>

#include <cstdint>
#include <cstdlib>
#include <string>
#include <utility>
#include <variant>
#include <vector>

/**
 * @brief Pack a single SenML record with a floating-point value (v).
 *
 * Encodes a CBOR array with one map: [{ 0: <name>, 2: <float64> }].
 */
std::vector<uint8_t> SenMLCborPacker::make(const std::string &name, double value)
{
    cbor_item_t *map = new_map_with_name_(name, /*pairs=*/2);
    cbor_map_add(map, (cbor_pair){
                          .key   = cbor_build_uint8(2),
                          .value = cbor_build_float8(value),  // float64
                      });
    return pack_one_record_(map);
}

/**
 * @brief Pack a single SenML record with an unsigned integer value (v).
 *
 * Encodes a CBOR array with one map: [{ 0: <name>, 2: <uint32_t> }].
 */
std::vector<uint8_t> SenMLCborPacker::make(const std::string &name, uint32_t value)
{
    cbor_item_t *map = new_map_with_name_(name, 2);
    cbor_map_add(map, (cbor_pair){
                          .key   = cbor_build_uint8(2),
                          .value = cbor_build_uint32(value),
                      });
    return pack_one_record_(map);
}

/**
 * @brief Pack a single SenML record with a boolean value (vb).
 *
 * Encodes a CBOR array with one map: [{ 0: <name>, 4: <bool> }].
 */
std::vector<uint8_t> SenMLCborPacker::make(const std::string &name, bool value)
{
    cbor_item_t *map = new_map_with_name_(name, 2);
    cbor_map_add(map, (cbor_pair){
                          .key   = cbor_build_uint8(4),
                          .value = cbor_build_bool(value),
                      });
    return pack_one_record_(map);
}

/**
 * @brief Pack a single SenML record with a string value (vs).
 *
 * Encodes a CBOR array with one map: [{ 0: <name>, 3: <string> }].
 */
std::vector<uint8_t> SenMLCborPacker::make(const std::string &name, const std::string &value)
{
    cbor_item_t *map = new_map_with_name_(name, 2);
    cbor_map_add(map, (cbor_pair){
                          .key   = cbor_build_uint8(3),
                          .value = cbor_build_string(value.c_str()),
                      });
    return pack_one_record_(map);
}

/**
 * @brief Pack multiple SenML records with double values into a single CBOR array.
 *
 * Each entry is packed as [{ 0: <name>, 2: <value> }, ...].
 *
 * Example usage:
 * auto payload = SenMLCborPacker::make_multiple({
 *     {"home_luminosity", home_lum},
 *     {"home_temperature", home_temp},
 *     {"home_humidity", home_hum}
 * });
 */
std::vector<uint8_t> SenMLCborPacker::make_multiple(
    const std::vector<std::pair<std::string, double>> &items)
{
    cbor_item_t *arr = cbor_new_definite_array(items.size());

    for (const auto &[name, value] : items)
    {
        cbor_item_t *map = new_map_with_name_(name, 2);

        cbor_map_add(map, (cbor_pair){
                              .key   = cbor_build_uint8(2),
                              .value = cbor_build_float8(value),
                          });

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

/**
 * @brief Pack a single SenML record with a C-string value (vs).
 *
 * Convenience overload that forwards to the std::string version.
 * Null pointers are converted to an empty string.
 */
std::vector<uint8_t> SenMLCborPacker::make(const std::string &name, const char *value)
{
    return make(name, std::string(value ? value : ""));
}

/**
 * @brief Create a CBOR map and set the SenML name field (key 0).
 *
 * Allocates a definite CBOR map with the requested number of pairs and
 * inserts {0: <name>} into it.
 */
cbor_item_t *SenMLCborPacker::new_map_with_name_(const std::string &name, size_t pairs)
{
    cbor_item_t *map = cbor_new_definite_map(pairs);

    // 0 -> name (n)
    cbor_map_add(map, (cbor_pair){
                          .key   = cbor_build_uint8(0),
                          .value = cbor_build_string(name.c_str()),
                      });

    return map;
}

/**
 * @brief Pack one record: CBOR array[ map{...} ]
 */
std::vector<uint8_t> SenMLCborPacker::pack_one_record_(cbor_item_t *map)
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
