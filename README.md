## Set up Docker

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

Obtain version:
```sh
arm-linux-gnueabihf-g++ --version
arm-linux-gnueabihf-g++ (Debian 14.2.0-19) 14.2.0
Copyright (C) 2024 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

## Sync SYSROOT

Create SYSROOT dir like `rpi-sysroot`:

```sh
cd ~
mkdir rpi-sysroot/
```

In Docker, set your `SYSROOT` directory, for example:
```sh
export SYSROOT=/rpi-env/rpi-sysroot
```

## Build project

The project use Paho MQTT client (both C and C++). You need to build and install them into `SYSROOT`, or build and rsync on RPi side. To build it on Host using Docker and copy it in `SYSROOT` dir use the below steps.

###  paho.mqtt.c

In Docker, set your `SYSROOT` directory if you did not::
```sh
export SYSROOT=/rpi-env/rpi-sysroot
```

And build lib:

```sh
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=../toolchain-rpi.cmake \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DPAHO_WITH_SSL=TRUE \
  -DPAHO_BUILD_SHARED=TRUE
cmake --build build -j
DESTDIR="$SYSROOT" cmake --install build
cd ..
```

### paho.mqtt.cpp

```sh
git clone https://github.com/eclipse/paho.mqtt.cpp.git
cd paho.mqtt.cpp
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=../toolchain-rpi.cmake \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DCMAKE_PREFIX_PATH="$SYSROOT/usr"
cmake --build build -j
DESTDIR="$SYSROOT" cmake --install build
```

## Save Docker

Obtain:
```sh
docker ps

CONTAINER ID   IMAGE                           COMMAND       CREATED              STATUS              PORTS     NAMES
4eab2bb77eb5   debian:trixie                   "bash"        About a minute ago   Up About a minute             tender_austin
f08498b58b84   custom_sdr_angel__last:latest   "/bin/bash"   7 weeks ago          Up 28 hours                   sdrangel-docker-antsdr
```

Save:
```sh
docker commit 4eab2bb77eb5 rpi-cross:trixie-ready
```

And run later as:
```sh
docker run -it rpi-cross:trixie-ready
```