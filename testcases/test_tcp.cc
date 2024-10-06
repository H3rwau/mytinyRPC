#include <memory>
#include "tinyRPC/common/log.h"
#include "tinyRPC/common/config.h"
#include "tinyRPC/net/tcp/net_addr.h"
#include "tinyRPC/net/tcp/tcp_server.h"

void test_tcp_server()
{
    tinyRPC::IPV4NetAddr::s_ptr addr = std::make_shared<tinyRPC::IPV4NetAddr>("127.0.0.1", 12345);
    DEBUGLOG("create addr %s", addr->toString().c_str());

    tinyRPC::TcpServer tcp_server(addr);
    tcp_server.start();
}

int main()
{
    tinyRPC::Config::setConfigPath("../conf/config.xml");
    tinyRPC::Logger::setLogLevel(tinyRPC::Config::getInstance()->m_log_level);

    test_tcp_server();
    return 0;
}