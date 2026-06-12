# Arduino Cloud C++ API

This library is designed for Linux-based devices to interact with the Arduino Cloud REST API and can be accessed through a set of endpoints to manage Devices, Things, Properties and more.

It serves as a `C++` alternative to the officially provided `Javascript`, `Python`, and `Golang` Arduino Cloud API clients.

## Module Layout

The application code should use this library through `ArduinoCloud` class. MQTT,
Paho, TLS setup, topics, publish counters, and CBOR/SenML packing stay inside
the library.

```text
src/client/
  ArduinoCloud.hpp/.cpp    Public library entry point

src/mqtt/
  MqttClient.hpp/.cpp      Paho MQTT adapter and connection handling

src/cborp/
  CborPacker.hpp/.cpp      SenML CBOR payload packing

src/types/
  Thing.hpp                Arduino IoT Cloud Thing topic helper
  PropertyUpdate.hpp       Property update API types
```

## Getting Started

### Get Arduino Cloud Credentials

The library awaits file `credentials.json` with the following structure:

```
{
  "device_id": "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx",
  "secret_key": "!1234567890QWERTY",
  "thing_id": "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
}
```

The `Device` itself it `Manually configured device`, like used for `Javascript`, `Python`, and `Golang` options. Other credentials obtained when registering and creating `Thing(s)`.

### Dependencies

The CMake target currently expects these dependencies to be available:

- CMake 3.16+
- C++17 compiler
- OpenSSL
- Threads
- libcbor
- Eclipse Paho MQTT C++

The right installation of it described in the next sections.

### Build Library

When cross-compiling for Raspberry Pi, install `libcbor` and Paho into the
configured `SYSROOT`. This module's CMake searches for `cbor.h` and `libcbor`
under `$SYSROOT/usr`.

#### Prepare Raspberry Pi SYSROOT

Create a local sysroot directory on the host:

```sh
mkdir -p "$HOME/rpi-sysroot"
```

Install base development packages on the Raspberry Pi before syncing:

```sh
ssh asist@raspberrypi.local
sudo apt update
sudo apt install -y libcbor-dev libssl-dev
exit
```

Sync the required Raspberry Pi system headers and libraries into it:

```sh
export RPI_HOST=asist@raspberrypi.local

rsync -a --delete "$RPI_HOST:/lib" "$HOME/rpi-sysroot/"
rsync -a --delete "$RPI_HOST:/usr/include" "$HOME/rpi-sysroot/usr/"
rsync -a --delete "$RPI_HOST:/usr/lib" "$HOME/rpi-sysroot/usr/"
```

The Docker commands below mount `$HOME` as `/rpi-env`, so the sysroot created at
`$HOME/rpi-sysroot` is visible inside Docker as `/rpi-env/rpi-sysroot`.

#### Docker build environment

Run docker:

```sh
docker run --rm -it \
  -v $HOME:/rpi-env \
  -w /rpi-env \
  debian:trixie bash
```

In Docker:

```sh
apt update

apt install -y cmake ninja-build pkg-config git \
  gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf \
  ca-certificates
```

And set your `SYSROOT` directory as:

```sh
export SYSROOT=/rpi-env/rpi-sysroot
```

#### Install Paho libraries

To build Paho for Raspberry Pi use the same `toolchain-rpi.cmake` as the parent
project, like:

```
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# IMPORTANT: use host-installed cross compiler, NOT sysroot/bin
set(CMAKE_C_COMPILER   /usr/bin/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/arm-linux-gnueabihf-g++)

set(CMAKE_SYSROOT //rpi-env/rpi-sysroot)
set(CMAKE_FIND_ROOT_PATH //rpi-env/rpi-sysroot)

# Search headers/libs in sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Optional: help find /usr/local in sysroot
list(APPEND CMAKE_PREFIX_PATH
  //rpi-env/rpi-sysroot/usr/local
  //rpi-env/rpi-sysroot/usr
)
```

#### paho.mqtt.c

Clone and install from sources `paho.mqtt.c`:

```sh
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=<path/to/toolchain-rpi.cmake> \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DPAHO_WITH_SSL=TRUE \
  -DPAHO_BUILD_SHARED=TRUE
cmake --build build -j
DESTDIR="$SYSROOT" cmake --install build
cd ..
```

#### paho.mqtt.cpp

Clone and install from sources `paho.mqtt.cpp`:

```sh
git clone https://github.com/eclipse/paho.mqtt.cpp.git
cd paho.mqtt.cpp
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=<path/to/toolchain-rpi.cmake> \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DCMAKE_PREFIX_PATH="$SYSROOT/usr"
cmake --build build -j
DESTDIR="$SYSROOT" cmake --install build
```

After these steps, the library can be consumed from CMake. Navigate to the `examples/` folder and try any example you like.

## Basic Usage

Create an `ArduinoCloud` instance, configure the target `Thing` with `Variables` (Floating Point Number) like **cpu_temperature** and **home_humidity** with mock values as demo. Publish it with interval (60 seconds here), and provide a callback returning mentioned property updates that will be synchronized with Arduino IoT Cloud:

```cpp
#include "ArduinoCloud.hpp"

ArduinoCloud cloud(device_id, device_secret);

cloud.addThing(thing_id);
cloud.setPeriod(60);
cloud.start();

cloud.sync([] {
    return PropertyUpdates
    {
        {"cpu_temperature", 52.4},
        {"home_humidity", 38.0},
    };
});

// cloud.stop();
```

The `sync()` method checks the configured period before collecting and publishing
properties. It is meant to be called regularly from the application loop.

The `publish()` can be used when the caller already has a list of property updates
and wants to send it immediately:

```cpp
cloud.publish({
    {"home_temperature", 23.5},
    {"status", std::string{"ready"}},
});
```

## Property Values

Use `PropertyUpdate` / `PropertyUpdates` to describe data sent to Arduino IoT
Cloud:

```cpp
using PropertyUpdates = std::vector<PropertyUpdate>;
```

Supported input values:

- `double`
- `float` packed as `double`
- `int` packed as `uint32_t` when non-negative, otherwise as `double`
- `uint32_t`
- `bool`
- `std::string`
- `const char *`

## Public API

```cpp
class ArduinoCloud
{
public:
    ArduinoCloud(const std::string &device_id,
                 const std::string &device_secret);
    ~ArduinoCloud();

    using PropertyReader = std::function<PropertyUpdates()>;

    void start();
    void stop();
    void addThing(const std::string &thing_id);
    void setPeriod(int seconds);
    void sync(const PropertyReader &reader);
    void publish(const PropertyUpdates &updates);
};
```

## CMake Usage notes

From the parent project you can add library as:

```cmake
add_subdirectory(libs/ArduinoIOTCloud)

target_link_libraries(my_app PRIVATE
  ArduinoIOTCloud::ArduinoIOTCloud
)
```

The library exposes public include paths for:

- `src/client`
- `src/types`

The MQTT, CBOR, and logger internals are private include paths.

## Developer Notes

- MQTT topics are generated from Things as `/a/t/<thing_id>/e/o`.
- Paho MQTT details are isolated in `src/mqtt/MqttClient.*`.
- CBOR payload generation is isolated in `src/cborp/CborPacker.*`.
- The application layer should prepare property values and call `sync()` or
  `publish()`; it should not create MQTT messages directly.
