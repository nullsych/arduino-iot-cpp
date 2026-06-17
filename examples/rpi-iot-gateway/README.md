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

TODO: picture.
TODO: picture.
TODO: picture.

## Scalability and Potential Extensions

It is easy to see that instead of a single simple device (like **ATmega328P**), a Raspberry Pi can act as a hub for multiple connected nodes that do not have their own Internet connection. While this sample project intentionally demonstrates a very simple setup, the same architecture can be easily extended into a much more capable IoT gateway!

Instead of communicating with a single ATmega328P over UART, it can collect data from sensors and microcontrollers connected through buses and industrial protocols such as **UART**, **I2C**, **SPI**, **RS-485**, or **Modbus**. This makes it possible to gather information from devices distributed across multiple rooms or floors and forward the collected data to network services.

This makes it possible to aggregate data from many sources, process it locally, and publish it to Arduino IoT Cloud (as simple example) through a single gateway. The approach scales from a small hobby project to larger home automation or monitoring, while keeping edge devices simple and focused on their primary tasks.

## Build

For cross-compilation setup see main [README.md](../../README.md) with corresponding section. The ArduinoIOTCloud library build as a static here.

After that, to configure the project use such example:

```sh
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=toolchain-rpi.cmake \
  -DPKG_CONFIG_EXECUTABLE="$(which pkg-config)" \
  -DPahoMqttCpp_DIR="$SYSROOT/usr/local/lib/cmake/PahoMqttCpp" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_DAEMON=ON
```

Build the application:

```sh
cmake --build build -j
```

The `BUILD_DAEMON=ON` option enables `systemd` integration and daemon.

## Deploy to Raspberry Pi

Copy the executable and credentials file to the target device:

```sh
scp examples/rpi-iot-gateway/build/arduino_iot_cloud_client pi@192.168.0.105:/home/pi
scp credentials.json pi@192.168.0.105:/home/pi
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

Reload systemd after creating or modifying the service:

```bash
sudo systemctl daemon-reload
```

Start the service:

```bash
sudo systemctl start arduino-iot
```

Check service status:

```bash
sudo systemctl status arduino-iot
```

Enable automatic startup on boot:

```bash
sudo systemctl enable arduino-iot
```

Stop the service:

```bash
sudo systemctl stop arduino-iot
```

Disable automatic startup:

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

Example output:

TODO