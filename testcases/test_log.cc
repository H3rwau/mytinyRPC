#include "../tinyRPC/common/log.h"
#include "../tinyRPC/common/config.h"

#include <thread>
#include <chrono>
#include <iostream>

void func(int x)
{
    DEBUGLOG("test log %s %d in thread", "2", x);
    INFOLOG("test info log %s %d in thread", "02", x);
    DEBUGLOG("test log %s %d in thread", "12", x);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    DEBUGLOG("test log %s %d in thread", "22", x);
    return;
}

int main()
{
    tinyRPC::Config::setConfigPath("../conf/config.xml");
    tinyRPC::Logger::setLogLevel(tinyRPC::Config::getInstance()->m_log_level);
    std::thread mythread(func, 2);
    DEBUGLOG("test debug log %s %d", "1", 888);
    INFOLOG("test info log %s", "6");
    INFOLOG("test info log %s", "4");
    INFOLOG("test info log %s", "5");
    DEBUGLOG("test debug log %s %d", "7", 999);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    INFOLOG("test info log %s", "5");
    DEBUGLOG("test debug log %s %d", "7", 999);
    mythread.join();

    return 0;
}