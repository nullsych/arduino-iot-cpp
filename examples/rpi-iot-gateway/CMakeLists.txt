cmake_minimum_required(VERSION 3.16)
project(arduino_iot_cloud_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_DAEMON "Build as daemon" OFF)

if(BUILD_DAEMON)
    add_definitions(-DBUILD_DAEMON)
endif()

# ---- Find packages ----
find_package(Threads REQUIRED)

add_subdirectory(libs/ArduinoIOTCloud)

file(GLOB_RECURSE MAIN_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(arduino_iot_cloud_client
  ${MAIN_SOURCES}
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
  src/MeteoStationUart/
)

target_link_libraries(arduino_iot_cloud_client PRIVATE
    ArduinoIOTCloud::ArduinoIOTCloud
    Threads::Threads
    -lboost_filesystem
    -lboost_system
)

target_compile_definitions(arduino_iot_cloud_client
	PRIVATE
	APPL_VERSION="${APPL_VERSION}"
)
