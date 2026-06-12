# RPi IoT Gateway

Example of `# C++ ArduinoIOTCloud` with RPi systemd application.

## Dependencies

The CMake target currently expects these dependencies to be available:

- CMake 3.16+
- C++17 compiler
- OpenSSL
- Threads
- [C++ ArduinoIOTCloud library](https://openai.com)

The last section described below.

### Install C++ ArduinoIOTCloud dependencies

Init `C++ ArduinoIOTCloud` as submodule:

```sh
git submodule update --init --recursive
```

And then follow [C++ ArduinoIOTCloud library](...) to install it's dependencies, and `SYSROOT` as well (if you do cross-compilation).

## Build & Sync

To have an option to run application as daemon, put `BUILD_DAEMON=ON` flag as well (if you do cross-compilation):

```sh
export SYSROOT=//rpi-env/rpi-sysroot
export PKG_CONFIG_SYSROOT_DIR="$SYSROOT"
export PKG_CONFIG_LIBDIR="$SYSROOT/usr/lib/arm-linux-gnueabihf/pkgconfig:$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/local/lib/pkgconfig"

cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=toolchain-rpi.cmake \
  -DPKG_CONFIG_EXECUTABLE="$(which pkg-config)" \
  -DPahoMqttCpp_DIR="$SYSROOT/usr/local/lib/cmake/PahoMqttCpp" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_DAEMON=ON
```

And after that deploy it into your RPi:

```sh
scp build/arduino_iot_cloud_client pi@192.168.0.105:/home/assist
```

## Prepare systemd unit

In new file, e.g. `arduino-iot.service`:

```bash
sudo nano /etc/systemd/system/arduino-iot.service
```

Put:

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
ExecStart=/path/to/arduino_iot_cloud_client
WorkingDirectory=/usr/local/bin
Restart=always
RestartSec=5
User=asist
Environment=HOME=/home/assist

# ttyUSB access group (optional)
Group=dialout

[Install]
WantedBy=multi-user.target
```

The `ExecStart` path can be `/home/assist/arduino_iot_cloud_client` on RPi.

The `credentials.json` should be in:

```
/usr/local/bin/credentials.json
```

---

# 🔧 2. Restart systemd

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

## See logs on device

Via `journalctl`:

```sh
sudo journalctl -f | grep "arduino"

Feb 16 18:47:02 raspberrypi arduino_iot_cloud_client[1000]: CPU temperature 57.996 C, Home temperature: 26.06 C, Home humidity = 25.3 %
Feb 16 18:47:02 raspberrypi arduino_iot_cloud_client[1000]: Published 7 packet(s) / Failed 0 packet(s)
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
