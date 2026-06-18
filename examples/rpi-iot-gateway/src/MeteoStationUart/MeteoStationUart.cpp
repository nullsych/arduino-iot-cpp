#include "MeteoStationUart.hpp"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <filesystem>
#include <iostream>
#include <regex>

#include "common/logs.h"

namespace fs = std::filesystem;

static int map_to_percent(int value, int min, int max)
{
    if (value <= min)
        return 0;

    if (value >= max)
        return 100;

    return ((value - min) * 100) / (max - min);
}

MeteoStationUart::MeteoStationUart(const std::string &port, int baudrate)
    : m_port(port), m_baudrate(baudrate)
{
}

MeteoStationUart::~MeteoStationUart()
{
    stop();
}

bool MeteoStationUart::isAlive()
{
    return m_alive;
}

bool MeteoStationUart::start()
{
    if (m_running)
        return true;

    auto findPort = [this]() -> std::string
    {
        if (fs::exists(m_port))
            return m_port;

        /* here we try ttyUSB first */
        for (const auto &entry : fs::directory_iterator("/dev"))
        {
            auto name = entry.path().filename().string();
            if (name.rfind("ttyUSB", 0) == 0)  // starts_with
            {
                return "/dev/" + name;
            }
        }

        /* if not - then ttyACM */
        for (const auto &entry : fs::directory_iterator("/dev"))
        {
            auto name = entry.path().filename().string();
            if (name.rfind("ttyACM", 0) == 0)
            {
                return "/dev/" + name;
            }
        }

        return {};
    };

    std::string detected = findPort();

    if (detected.empty())
    {
        return false;
    }

    m_port  = detected;
    m_alive = true;
    LOG_INF("Port found: " << m_port);

    m_running = true;
    m_thread  = std::thread(&MeteoStationUart::readerLoop, this);
    return true;
}

void MeteoStationUart::stop()
{
    m_running = false;
    if (m_thread.joinable())
        m_thread.join();
}

double MeteoStationUart::getTemperature() const
{
    return m_temp.load();
}

double MeteoStationUart::getHumidity() const
{
    return m_hum.load();
}

int MeteoStationUart::getAirPollutionLevel() const
{
    int air_polution_level = 0;
    auto mq135_raw         = m_mq135.load();
    const double temp      = m_temp.load();
    const double hum       = m_hum.load();

    // 20 C and  65% RH as basis
    air_polution_level = mq135_raw * (1.0 + (temp - 30.0) * 0.01);
    air_polution_level = mq135_raw * (1.0 + (65.0 - hum) * 0.005);

    int percent =
        static_cast<int>((air_polution_level - m_clean_air) * 100.0 / (m_dirty_air - m_clean_air));

    return std::clamp(percent, 0, 100);
}

int MeteoStationUart::getCoLevel() const
{
    int co_level = 0;
    auto mq7_raw = m_mq7.load();

    return map_to_percent(mq7_raw, m_clean_air, m_dirty_air);
}

int MeteoStationUart::getStationUptime() const
{
    return m_time_s.load();
}

void MeteoStationUart::readerLoop()
{
    int fd = open(m_port.c_str(), O_RDONLY | O_NOCTTY);
    if (fd < 0)
    {
        perror("open serial");
        return;
    }

    termios tty{};
    tcgetattr(fd, &tty);

    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;

    tcsetattr(fd, TCSANOW, &tty);

    std::regex re(
        R"(Temp:\s*([0-9.]+)\s*C,\s*Hum:\s*([0-9.]+)\s*%,\s*MQ135:\s*([0-9]+),\s*MQ7:\s*([0-9]+).*?t,\s*s:\s*([0-9]+))");

    std::string buffer;
    buffer.reserve(256);

    char ch;
    while (m_running && read(fd, &ch, 1) == 1)
    {
        if (ch == '\n')
        {
            std::smatch match;
            if (std::regex_search(buffer, match, re))
            {
                try
                {
                    double t   = std::stod(match[1]);
                    double h   = std::stod(match[2]);
                    int mq135  = std::stoi(match[3]);
                    int mq7    = std::stoi(match[4]);
                    int time_s = std::stoi(match[5]);

                    m_temp     = t;
                    m_hum      = h;
                    m_mq135    = mq135;
                    m_mq7      = mq7;
                    m_time_s   = time_s;
                }
                catch (...)
                {
                    // Игнорируем ошибки конвертации, если вдруг пришел мусор
                }
            }
            buffer.clear();
        }
        else
        {
            buffer += ch;
        }
    }

    close(fd);
}