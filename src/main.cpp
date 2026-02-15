#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <csignal>
#include <iostream>
#include <string>

#include "Application.hpp"
#include "common/logs.h"

Application *app_ptr = nullptr;

void signal_handler(int sig)
{
    if (sig == SIGINT || sig == SIGTERM)
    {
        if (app_ptr)
            app_ptr->stop();
    }
}

int main()
{
#ifdef BUILD_DAEMON
    logger::init_syslog("arduino_iot_cloud_client");
#endif

#ifdef BUILD_DAEMON
    logger::close_syslog();
#endif

    boost::property_tree::ptree pt;
    boost::property_tree::read_json("credentials.json", pt);

    if (!boost::filesystem::exists("credentials.json"))
    {
        LOG_ERROR("File credentials.json not found! Create one and run again.");
        return -1;
    }

    std::string device_id  = pt.get<std::string>("device_id");
    std::string secret_key = pt.get<std::string>("secret_key");
    std::string thing_id   = pt.get<std::string>("thing_id");

    Application app(device_id, secret_key);
    app_ptr = &app;

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    app.addThing(thing_id);
    app.setPeriod(30 * 1);  // 1 minute

    app.start();

    return 0;
}
