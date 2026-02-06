cmake_minimum_required(VERSION 3.16)
project(home_assist_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

file(GLOB SRC_FILES src/*.cpp)

add_executable(home_assist_client ${SRC_FILES})

include_directories(${CMAKE_SYSROOT}/usr/include/arm-linux-gnueabihf)
link_directories(${CMAKE_SYSROOT}/usr/lib/arm-linux-gnueabihf)

# target_link_libraries(home_assist_client CURL::libcurl)
target_link_libraries(home_assist_client curl ssl crypto z)
