#include <iostream>
#include <string>

#include "Application.hpp"

#define DEVICE_ID "e788959f-1cd6-4c9f-a474-64ffcff6d966"
#define SECRET_KEY "!RH7GzTmC5mSwMIMA1XH?J@RD"
#define THING_ID "338c584c-a406-4de3-9671-23a798515ce8"

int main()
{
#ifdef BUILD_DAEMON
    logger::init_syslog("mydaemon");
#endif

    // LOG_INFO("Service started");
    // LOG_ERROR("Error code " << 42);

#ifdef BUILD_DAEMON
    logger::close_syslog();
#endif

    Application server_app(DEVICE_ID, SECRET_KEY);
    server_app.addThing(THING_ID);
    server_app.start();

    return 0;
}
