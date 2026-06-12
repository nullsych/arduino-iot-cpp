#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <filesystem>
#include <iostream>
#include <regex>

#include "MeteoStationUart.hpp"
#include "common/logs.h"

namespace fs = std::filesystem;

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

    std::regex re(R"(Temp:\s*([0-9.]+)\s*C,\s*Hum:\s*([0-9.]+))");

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
                    double t = std::stod(match[1]);
                    double h = std::stod(match[2]);

                    m_temp   = t;
                    m_hum    = h;
                }
                catch (...)
                {
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
