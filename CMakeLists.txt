cmake_minimum_required(VERSION 3.16)
project(ArduinoIOTCloud VERSION 0.1.0 LANGUAGES CXX)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(PahoMqttCpp CONFIG REQUIRED)

find_path(CBOR_INCLUDE_DIR cbor.h
  PATHS
    "$ENV{SYSROOT}/usr/include"
    "$ENV{SYSROOT}/usr/local/include"
  NO_DEFAULT_PATH
)

find_library(CBOR_LIBRARY cbor
  PATHS
    "$ENV{SYSROOT}/usr/lib/arm-linux-gnueabihf"
    "$ENV{SYSROOT}/usr/local/lib"
    "$ENV{SYSROOT}/usr/lib"
  NO_DEFAULT_PATH
)

if(NOT CBOR_INCLUDE_DIR OR NOT CBOR_LIBRARY)
  message(FATAL_ERROR "libcbor not found in sysroot")
endif()

add_library(ArduinoIOTCloud STATIC
  src/client/ArduinoCloud.cpp
  src/cborp/CborPacker.cpp
  src/mqtt/MqttClient.cpp
)

add_library(ArduinoIOTCloud::ArduinoIOTCloud ALIAS ArduinoIOTCloud)

target_compile_features(ArduinoIOTCloud PUBLIC cxx_std_17)

target_include_directories(ArduinoIOTCloud
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/client
    ${CMAKE_CURRENT_SOURCE_DIR}/src/types
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cborp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mqtt
    ${CBOR_INCLUDE_DIR}
)

target_link_libraries(ArduinoIOTCloud
  PRIVATE
    PahoMqttCpp::paho-mqttpp3
    ${CBOR_LIBRARY}
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)
