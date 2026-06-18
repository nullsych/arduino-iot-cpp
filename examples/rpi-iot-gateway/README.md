# RPi IoT Gateway

My home automation project that demonstrates a simple IoT gateway built from two components:

* An **ATmega328P-based Home Weather Station** equipped with a **BME280** environmental sensor, as well as **MQ135** & **MQ7**.
* A **Raspberry Pi** running the Arduino IoT Cloud C++ client.

The BME280 sensor periodically measures temperature and humidity, while the ATmega328P sends the collected data to the Raspberry Pi over a USB serial (UART) connection:

```text
Temp: 27.64 C, Hum: 39.3 %, MQ135: 103, MQ7: 436  |  e72b85b  ### SYS OK @ (MQ ARM) t, s: 60
Temp: 27.64 C, Hum: 39.3 %, MQ135: 103, MQ7: 436  |  e72b85b  ### SYS OK @ (MQ ARM) t, s: 60
Temp: 27.64 C, Hum: 39.3 %, MQ135: 103, MQ7: 434  |  e72b85b  ### SYS OK @ (MQ ARM) t, s: 61
Temp: 27.65 C, Hum: 39.3 %, MQ135: 103, MQ7: 433  |  e72b85b  ### SYS OK @ (MQ ARM) t, s: 61
```

The Raspberry Pi acts as an IoT gateway. It receives sensor readings from the weather station, parses the incoming UART data, and publishes the measurements to `Arduino IoT Cloud` using a `manually registered device`.

As a result temperature, humidity and air qualities measurements collected by the weather station can be viewed and monitored remotely through my Arduino IoT Cloud dashboard:

![dashboard.png](dashboard.png)

## Scalability and Potential Extensions

It is easy to see that instead of a single simple device (like **ATmega328P**), a Raspberry Pi can act as a hub for multiple connected nodes that do not have their own Internet connection. While this sample project intentionally demonstrates a very simple setup, the same architecture can be easily extended into a much more capable IoT gateway!

Instead of communicating with a single ATmega328P over UART, it can collect data from sensors and microcontrollers connected through buses and industrial protocols such as **UART**, **I2C**, **SPI**, **RS-485**, or **Modbus**. This makes it possible to gather information from devices distributed across multiple rooms or floors and forward the collected data to network services.

This makes it possible to aggregate data from many sources, process it locally, and publish it to Arduino IoT Cloud (as simple example) through a single gateway. The approach scales from a small hobby project to larger home automation or monitoring, while keeping edge devices simple and focused on their primary tasks.

## Build

For cross-compilation setup see main [README.md](../../README.md) with corresponding section. The ArduinoIOTCloud library build as a static here.

After that, to configure the project, if you inside this repo, use such example:

```sh
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=../toolchain-rpi.cmake \
  -DPKG_CONFIG_EXECUTABLE="$(which pkg-config)" \
  -DPahoMqttCpp_DIR="$SYSROOT/usr/local/lib/cmake/PahoMqttCpp" \
  -DCMAKE_BUILD_TYPE=Release \
  -DARDUINO_IOT_CLOUD_DIR="../../" \
  -DBUILD_DAEMON=ON
```

Build the application:

```sh
cmake --build build -j$(nproc)
```

The `BUILD_DAEMON=ON` option enables `systemd` integration and daemon.

## Deploy to Raspberry Pi

Copy the executable and credentials file to the target device:

```sh
scp examples/rpi-iot-gateway/build/arduino_iot_cloud_client pi@192.168.0.105:/home/pi
scp credentials.json pi@192.168.0.105:/home/pi
```

Example output if run as non-daemon application:

```
pi@raspberrypi:~ $ ./arduino_iot_cloud_client 

[19:34:49] (INFO)  Git version: db5044b
[19:34:49] (INFO)  Port found: /dev/ttyUSB0
[19:34:49] (INFO)  Starting MQTT client at: ssl://iot.arduino.cc:8884
[19:34:50] (INFO)  MQTT connected
[19:34:50] (INFO)  CPU: 57.996 C
[19:34:50] (INFO)  Home temperature: 30.72 C, Home humidity: 33.3%, CO level: 11%, Pollution level 0%
[19:34:50] (INFO)  Published 1 packet(s) / Failed 0 packet(s)
[19:35:00] (INFO)  CPU: 56.92 C
[19:35:00] (INFO)  Home temperature: 30.7 C, Home humidity: 33.8%, CO level: 11%, Pollution level 0%
[19:35:00] (INFO)  Published 2 packet(s) / Failed 0 packet(s)
[19:35:10] (INFO)  CPU: 56.92 C
[19:35:10] (INFO)  Home temperature: 30.7 C, Home humidity: 33.9%, CO level: 11%, Pollution level 0%
[19:35:11] (INFO)  Published 3 packet(s) / Failed 0 packet(s)
[19:35:21] (INFO)  CPU: 56.92 C
```

## Configure a Systemd Service

Create a new service file:

```bash
sudo nano /etc/systemd/system/arduino-iot.service
```

Add the following configuration:

```ini
[Unit]
Description=Arduino IoT Cloud Client
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
ExecStart=/usr/local/bin/arduino_iot_cloud_client
WorkingDirectory=/usr/local/bin
User=pi
Group=dialout
Environment=HOME=/home/pi
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

Relocate both `arduino_iot_cloud_client` and `credentials.json` in `usr/local/bin` as well.

### Notes

* Adjust `ExecStart` to match the actual location of the executable.
* The `dialout` group is only required when accessing serial devices such as `/dev/ttyUSB*`.
* Ensure that `credentials.json` is available at:

```text
/usr/local/bin/credentials.json
```

## Manage the Service

### Reload systemd after creating or modifying the service

```bash
sudo systemctl daemon-reload
```

### Start the service

```bash
sudo systemctl start arduino-iot
```

### Check service status

```bash
sudo systemctl status arduino-iot
```

Typical output:

```
● arduino-iot.service - Arduino IoT Cloud Client
     Loaded: loaded (/etc/systemd/system/arduino-iot.service; enabled; preset: enabled)
     Active: active (running) since Thu 2026-06-18 13:48:45 BST; 4min 19s ago
 Invocation: 65459ab4695444559e0b5fcc5c8b98cc
   Main PID: 778 (arduino_iot_clo)
      Tasks: 4 (limit: 1569)
        CPU: 914ms
     CGroup: /system.slice/arduino-iot.service
             └─778 /home/pi/arduino_iot_cloud_client

Jun 18 13:52:32 raspberrypi arduino_iot_cloud_client[778]: Published 22 packet(s) / Failed 0 packet(s)
Jun 18 13:52:42 raspberrypi arduino_iot_cloud_client[778]: CPU: 59.072 C
Jun 18 13:52:42 raspberrypi arduino_iot_cloud_client[778]: Home temperature: 31.36 C, Home humidity: 36.4%, CO level: 24%, Pollution level 0%
Jun 18 13:52:42 raspberrypi arduino_iot_cloud_client[778]: Published 23 packet(s) / Failed 0 packet(s)
Jun 18 13:52:52 raspberrypi arduino_iot_cloud_client[778]: CPU: 60.148 C
Jun 18 13:52:52 raspberrypi arduino_iot_cloud_client[778]: Home temperature: 31.35 C, Home humidity: 36.4%, CO level: 24%, Pollution level 0%
Jun 18 13:52:52 raspberrypi arduino_iot_cloud_client[778]: Published 24 packet(s) / Failed 0 packet(s)
Jun 18 13:53:02 raspberrypi arduino_iot_cloud_client[778]: CPU: 59.072 C
Jun 18 13:53:02 raspberrypi arduino_iot_cloud_client[778]: Home temperature: 31.36 C, Home humidity: 36.4%, CO level: 24%, Pollution level 0%
Jun 18 13:53:03 raspberrypi arduino_iot_cloud_client[778]: Published 25 packet(s) / Failed 0 packet(s)
```

### Enable automatic startup on boot:

```bash
sudo systemctl enable arduino-iot
```

### Stop the service

```bash
sudo systemctl stop arduino-iot
```

### Disable automatic startup

```bash
sudo systemctl disable arduino-iot
```

## View Logs

Follow service logs in real time:

```bash
journalctl -u arduino-iot -f
```

Or filter logs directly from the system journal:

```bash
sudo journalctl -f | grep arduino
```

Typical output:

```
Jun 18 13:53:34 raspberrypi arduino_iot_cloud_client[778]: Published 28 packet(s) / Failed 0 packet(s)
Jun 18 13:53:44 raspberrypi arduino_iot_cloud_client[778]: CPU: 59.072 C
Jun 18 13:53:44 raspberrypi arduino_iot_cloud_client[778]: Home temperature: 31.44 C, Home humidity: 35.5%, CO level: 21%, Pollution level 0%
Jun 18 13:53:44 raspberrypi arduino_iot_cloud_client[778]: Published 29 packet(s) / Failed 0 packet(s)
Jun 18 13:53:54 raspberrypi arduino_iot_cloud_client[778]: CPU: 59.072 C
Jun 18 13:53:54 raspberrypi arduino_iot_cloud_client[778]: Home temperature: 31.43 C, Home humidity: 35.7%, CO level: 21%, Pollution level 0%
Jun 18 13:53:54 raspberrypi arduino_iot_cloud_client[778]: Published 30 packet(s) / Failed 0 packet(s)
Jun 18 13:54:04 raspberrypi arduino_iot_cloud_client[778]: CPU: 59.072 C
Jun 18 13:54:04 raspberrypi arduino_iot_cloud_client[778]: Home temperature: 31.4 C, Home humidity: 36%, CO level: 22%, Pollution level 0%
Jun 18 13:54:05 raspberrypi arduino_iot_cloud_client[778]: Published 31 packet(s) / Failed 0 packet(s)
Jun 18 13:54:15 raspberrypi arduino_iot_cloud_client[778]: CPU: 59.61 C
```
