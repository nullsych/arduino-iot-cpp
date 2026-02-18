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
docker run -it -v $HOME:/rpi-env -w /rpi-env rpi-cross:trixie-ready
```


## See logs

Via `journalctl`:

```sh
sudo journalctl -f | grep "arduino"
Feb 16 18:47:02 raspberrypi arduino_iot_cloud_client[1000]: CPU temperature 57.996 C, Home temperature: 26.06 C, Home humidity = 25.3 %
Feb 16 18:47:02 raspberrypi arduino_iot_cloud_client[1000]: Published 7 packet(s) / Failed 0 packet(s)
```


Хрюша Соплюша 🐷, самый правильный способ автозапуска демона на Linux (и на твоей малине) — через **systemd service**.
Это стандарт и самый надёжный вариант.

---

# 🚀 1. Создаём systemd unit

Создай файл:

```bash
sudo nano /etc/systemd/system/arduino-iot.service
```

Вставь 👇

```ini
[Unit]
Description=Arduino IoT Cloud Client
After=network-online.target
Wants=network-online.target
Restart=always
RestartSec=5
StartLimitInterval=0


[Service]
Type=simple
ExecStart=/home/asist/arduino_iot_cloud_client
WorkingDirectory=/usr/local/bin
Restart=always
RestartSec=5
User=asist
Environment=HOME=/home/asist

# ttyUSB access group
Group=dialout

[Install]
WantedBy=multi-user.target
```


The `credentials.json` should be in:

```
/usr/local/bin/credentials.json
```

---

# 🔧 2. Перезагрузить systemd

```bash
sudo systemctl daemon-reload
```


Start (manual):

```bash
sudo systemctl start arduino-iot
```

Check:

```bash
sudo systemctl status arduino-iot

● arduino-iot.service - Arduino IoT Cloud Client
     Loaded: loaded (/etc/systemd/system/arduino-iot.service; disabled; preset: enabled)
     Active: activating (auto-restart) since Mon 2026-02-16 18:55:38 GMT; 159ms ago
 Invocation: 6502f1824b6d41e881a57f820ef72243
    Process: 1175 ExecStart=/home/asist/arduino_iot_cloud_client (code=exited, status=0/SUCCESS)
   Main PID: 1175 (code=exited, status=0/SUCCESS)
        CPU: 231ms
```

Enable autostart:

```bash
sudo systemctl enable arduino-iot
```

See logs:

```bash
journalctl -u arduino-iot -f
```

Stop:

```bash
sudo systemctl stop arduino-iot
```

Disable autostart:

```bash
sudo systemctl disable arduino-iot
```


For Arduino `/dev/ttyUSB0`:

```ini
Group=dialout
```

And add user:

```bash
sudo usermod -aG dialout asist
```



sudo systemctl start arduino-iot


sudo systemctl status arduino-iot
