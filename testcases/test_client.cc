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
#include "tinyRPC/common/log.h"
#include "tinyRPC/common/config.h"
#include "tinyRPC/net/tcp/tcp_client.h"
#include "tinyRPC/net/tcp/net_addr.h"
#include "tinyRPC/net/coder/string_coder.h"
#include "tinyRPC/net/coder/abstract_protocol.h"
#include "tinyRPC/net/coder/tinypb_coder.h"
#include "tinyRPC/net/coder/tinypb_protocol.h"

void test_connect()
{
    // 调用 conenct 连接 server
    // wirte 一个字符串
    // 等待 read 返回结果

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0)
    {
        ERRORLOG("invalid fd %d", fd);
        exit(0);
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_aton("127.0.0.1", &server_addr.sin_addr);

    int rt = connect(fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr));

    DEBUGLOG("connect success");

    std::string msg = "hello h3rwau!!!!!";

    rt = write(fd, msg.c_str(), msg.length());

    DEBUGLOG("success write %d bytes, [%s]", rt, msg.c_str());

    char buf[100];
    memset(buf, 0, sizeof(buf));
    rt = read(fd, buf, 100);
    DEBUGLOG("success read %d bytes, [%s]", rt, std::string(buf).c_str());
}

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
                       message->m_msg_id = "123456";
                       message->m_pb_data = "test client pb data";
                       // writeMessage中设置了可写事件监听
                       client.writeMessage(message, [](tinyRPC::AbstractProtocol::s_ptr msg_ptr)
                                           { DEBUGLOG("send message success"); });

                       client.readMessage("123456", [](tinyRPC::AbstractProtocol::s_ptr msg_ptr)
                                          {
        std::shared_ptr<tinyRPC::TinyPBProtocol> message = std::dynamic_pointer_cast<tinyRPC::TinyPBProtocol>(msg_ptr);
            DEBUGLOG("msg_id[%s], get response %s", message->m_msg_id.c_str(), message->m_pb_data.c_str()); }); });
}

int main()
{
    tinyRPC::Config::setConfigPath("../conf/config.xml");
    tinyRPC::Logger::setLogLevel(tinyRPC::Config::getInstance()->m_log_level);
    test_tcp_client();
    return 0;
}