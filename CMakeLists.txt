cmake_minimum_required(VERSION 3.16)
project(arduino_iot_cloud_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_DAEMON "Build as daemon" OFF)

if(BUILD_DAEMON)
    add_definitions(-DBUILD_DAEMON)
endif()

# ---- Find packages ----
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

file(GLOB_RECURSE MAIN_SOURCES src/*.cpp)
file(GLOB_RECURSE CBOR_PACKER_SOURCES src/SenMLCborPacker/*.cpp)
file(GLOB_RECURSE STATION_DATA_SOURCES src/Application/*.cpp)
file(GLOB_RECURSE APPLICATION_SOURCES src/Application/*.cpp)

add_executable(arduino_iot_cloud_client
  ${MAIN_SOURCES}
  ${CBOR_PACKER_SOURCES}
  ${APPLICATION_SOURCES}
  ${STATION_DATA_SOURCES}
)

# Try to get git commit hash (short)
execute_process(
  COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE APPL_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

if(APPL_VERSION STREQUAL "")
  set(APPL_VERSION "dev")
endif()

target_include_directories(arduino_iot_cloud_client
  PRIVATE
  src/
  src/Application
  src/common
  src/SenMLCborPacker/
  src/StationData/
  ${CBOR_INCLUDE_DIR}
)

target_link_directories(arduino_iot_cloud_client PRIVATE
    ${CBOR_LIBRARY_DIRS}
)

target_link_libraries(arduino_iot_cloud_client PRIVATE
    PahoMqttCpp::paho-mqttpp3
    ${CBOR_LIBRARY}
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    -lboost_filesystem
    -lboost_system
)

target_compile_definitions(arduino_iot_cloud_client
	PRIVATE
	APPL_VERSION="${APPL_VERSION}"
)
