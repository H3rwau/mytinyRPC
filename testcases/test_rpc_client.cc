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

void test_tcp_client()
{
    tinyRPC::IPV4NetAddr::s_ptr addr = std::make_shared<tinyRPC::IPV4NetAddr>("127.0.0.1", 12345);
    tinyRPC::TcpClient client(addr);
    // connect成功了的话会监听可写事件，
    // 在tcp的connection里的onwrite会做如下操作
    //  1. 将 message encode 得到字节流
    // 2. 将字节流入到 buffer 里面，然后全部发送
    client.connect([addr, &client]()
                   {
        DEBUGLOG("conenct to [%s] success", addr->toString().c_str());
        std::shared_ptr<tinyRPC::TinyPBProtocol> message = std::make_shared<tinyRPC::TinyPBProtocol>();
        message->m_req_id = "123456";
        message->m_pb_data = "test client pb data";

        makeOrderRequest req;
        req.set_price(100);
        req.set_goods("iphone16 Pro Max");

        if(!req.SerializeToString(&(message->m_pb_data))){
            ERRORLOG("serilize error");
            return;
        }
        message->m_method_name = "Order.makeOrder";

        //writeMessage中设置了可写事件监听
        client.writeMessage(message, [req](tinyRPC::AbstractProtocol::s_ptr msg_ptr)
                            {
            DEBUGLOG("send message success, request[%s]", req.ShortDebugString().c_str());; });

        client.readMessage("123456", [](tinyRPC::AbstractProtocol::s_ptr msg_ptr)
                           {
        std::shared_ptr<tinyRPC::TinyPBProtocol> message = std::dynamic_pointer_cast<tinyRPC::TinyPBProtocol>(msg_ptr);
            DEBUGLOG("req_id[%s], get response %s", message->m_req_id.c_str(), message->m_pb_data.c_str());
            makeOrderResponse response;
            if (!response.ParseFromString(message->m_pb_data))
            {
                ERRORLOG("deserialize error");
                return;
            }
            DEBUGLOG("get response success, response[%s]", response.ShortDebugString().c_str());
            });
        
                             });
}

int main()
{
    tinyRPC::Config::setConfigPath("../conf/config.xml");
    tinyRPC::Logger::setLogLevel(tinyRPC::Config::getInstance()->m_log_level);
    test_tcp_client();
    return 0;
}