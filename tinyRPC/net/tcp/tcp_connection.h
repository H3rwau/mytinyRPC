#ifndef __TCP_CONNECTION__
#define __TCP_CONNECTION__


#include <memory>
#include "tinyRPC/net/tcp/net_addr.h"
#include "tinyRPC/net/tcp/tcp_buffer.h"
#include "tinyRPC/net/io_thread.h"

namespace  tinyRPC{
    enum TcpState
    {
        NotConnected = 1,
        Connected = 2,
        HalfClosing = 3,
        Closed = 4,
    };
    enum TcpConnectionType
    {
        //作为服务端使用，表示和客户端的连接
        TcpConnectionByServer = 1,
        //作为客户端使用，表示和服务端的连接
        TcpConnectionByClient = 2,

    };
    class TcpConnection
    {
    public:
        using s_ptr = std::shared_ptr<TcpConnection>;

    public:
        TcpConnection(EventLoop * event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr);
        ~TcpConnection();
        void onRead();
        void excute();
        void onWrite();
        void setState(const TcpState state);
        TcpState getState();
        void clear();
        // 服务器主动关闭连接
        void shutdown();
        void setConnectionType(TcpConnectionType type);

    private:
        EventLoop *m_event_loop{NULL}; // 代表持有该连接的loop
        NetAddr::s_ptr m_local_addr;
        NetAddr::s_ptr m_peer_addr;
        TcpBuffer::s_ptr m_in_buffer;  // 接收缓冲区
        TcpBuffer::s_ptr m_out_buffer; // 发送缓冲区
        FdEvent *m_fd_event{NULL};
        TcpState m_state;
        int m_fd{0};
        TcpConnectionType m_connection_type{TcpConnectionByServer};
    };
}//tinyRPC

#endif // __TCP_CONNECTION__