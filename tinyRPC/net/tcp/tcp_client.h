#ifndef __TCP_CLIENT__
#define __TCP_CLIENT__

#include <memory>
#include "tinyRPC/net/tcp/net_addr.h"
#include "tinyRPC/net/eventloop.h"
#include "tinyRPC/net/tcp/tcp_connection.h"
#include "tinyRPC/net/coder/abstract_protocol.h"
#include "tinyRPC/net/timer_event.h"

namespace tinyRPC
{
    class TcpClient
    {
    public:
        using s_ptr = std::shared_ptr<TcpClient>;
        TcpClient(NetAddr::s_ptr peer_addr);
        ~TcpClient();

        // 异步的进行 conenct
        // 如果connect 成功，done 会被执行
        void connect(std::function<void()> done);
        // 异步的发送 message
        // 如果发送 message 成功，会调用 done 函数， 函数的入参就是 message 对象
        void writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);
        // 异步的读取 message
        // 如果读取 message 成功，会调用 done 函数， 函数的入参就是 message 对象
        void readMessage(const std::string &msg_id, std::function<void(AbstractProtocol::s_ptr)> done);
        void stop();

        int getConnectErrorCode();

        std::string getConnectErrorInfo();

        NetAddr::s_ptr getPeerAddr();

        NetAddr::s_ptr getLocalAddr();

        void initLocalAddr();
        void addTimerEvent(TimerEvent::s_ptr timer_event);

    private:
        NetAddr::s_ptr m_peer_addr;
        NetAddr::s_ptr m_local_addr;
        EventLoop *m_event_loop{NULL};
        int m_fd{-1};
        FdEvent *m_fd_event{NULL};
        TcpConnection::s_ptr m_connection;

        int m_connect_error_code{0};
        std::string m_connect_error_info;
    };
} // tinyRPC

#endif // __TCP_CLIENT__