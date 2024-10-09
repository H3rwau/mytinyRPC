#include <assert.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <thread>
#include <google/protobuf/service.h>
#include "tinyRPC/common/log.h"
#include "tinyRPC/common/config.h"
#include "tinyRPC/net/tcp/tcp_client.h"
#include "tinyRPC/net/tcp/net_addr.h"
#include "tinyRPC/net/coder/string_coder.h"
#include "tinyRPC/net/coder/abstract_protocol.h"
#include "tinyRPC/net/coder/tinypb_coder.h"
#include "tinyRPC/net/coder/tinypb_protocol.h"
#include "tinyRPC/net/tcp/tcp_server.h"
#include "tinyRPC/net/rpc/rpc_dispatcher.h"

#include "order.pb.h"

class OrderImpl : public Order
{
public:
    void makeOrder(google::protobuf::RpcController *controller,
                   const ::makeOrderRequest *request,
                   ::makeOrderResponse *response,
                   ::google::protobuf::Closure *done)
    {
        APPDEBUGLOG("start sleep 5s");
        sleep(5);
        APPDEBUGLOG("end sleep 5s");
        if (request->price() < 10)
        {
            response->set_ret_code(-1);
            response->set_res_info("short balance");
            return;
        }
        response->set_order_id("20241008");
        APPDEBUGLOG("call makeOrder success");
    }
};

void test_tcp_server()
{
    tinyRPC::IPV4NetAddr::s_ptr addr = std::make_shared<tinyRPC::IPV4NetAddr>("127.0.0.1", 12345);

    DEBUGLOG("create addr %s", addr->toString().c_str());
    tinyRPC::TcpServer tcp_server(addr);
    tcp_server.start();
}

int main(int argc,char ** argv)
{
    if (argc != 2)
    {
        printf("Start test_rpc_server error, argc not 2 \n");
        printf("Start like this: \n");
        printf("./test_rpc_server ../conf/rocket.xml \n");
        return 0;
    }

    tinyRPC::Config::setConfigPath(argv[1]);
    tinyRPC::Logger::setLogLevel(tinyRPC::Config::getInstance()->m_log_level);
    // printf("111111\n");
    tinyRPC::Logger::getInstance()->InitGlobalLogger(1);
    // printf("22222\n");

    std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    tinyRPC::RpcDispatcher::GetRpcDispatcher()->registerService(service);

    //test_tcp_server();
    tinyRPC::IPV4NetAddr::s_ptr addr = std::make_shared<tinyRPC::IPV4NetAddr>("127.0.0.1", tinyRPC::Config::getInstance()->m_port);
    DEBUGLOG("create addr %s", addr->toString().c_str());
    tinyRPC::TcpServer tcp_server(addr);
    tcp_server.start();
    return 0;
}