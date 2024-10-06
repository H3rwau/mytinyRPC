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
    class TcpConnection
    {
    public:
        using s_ptr = std::shared_ptr<TcpConnection>;

    public:
        TcpConnection(IOThread *io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr);
        ~TcpConnection();
        void onRead();
        void excute();
        void onWrite();
        void setState(const TcpState state);
        TcpState getState();
        void clear();
        // 服务器主动关闭连接
        void shutdown();

    private:
        IOThread *m_io_thread{NULL}; // 代表持有该连接的 IO 线程
        NetAddr::s_ptr m_local_addr;
        NetAddr::s_ptr m_peer_addr;
        TcpBuffer::s_ptr m_in_buffer;  // 接收缓冲区
        TcpBuffer::s_ptr m_out_buffer; // 发送缓冲区
        FdEvent *m_fd_event{NULL};
        TcpState m_state;
        int m_fd{0};
    };
}//tinyRPC

#endif // __TCP_CONNECTION__