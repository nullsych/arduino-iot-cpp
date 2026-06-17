#pragma once

#include <atomic>
#include <string>
#include <thread>

class MeteoStationUart
{
public:
    explicit MeteoStationUart(const std::string &port, int baudrate = 115200);
    ~MeteoStationUart();

    bool start();
    void stop();
    bool isAlive();

    double getTemperature() const;
    double getHumidity() const;
    int getMq135() const;
    int getMq7() const;
    int getStationUptime() const;

private:
    void readerLoop();

private:
    std::string m_port;
    bool m_alive = false;
    int m_baudrate;

    std::atomic<bool> m_running{false};
    std::thread m_thread;

    std::atomic<double> m_temp{0.0};
    std::atomic<double> m_hum{0.0};
    std::atomic<int> m_mq135{0};
    std::atomic<int> m_mq7{0};
    std::atomic<int> m_time_s{0};
};
