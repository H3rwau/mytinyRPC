#ifndef __TCP_SERVER__
#define __TCP_SERVER__

#include <set>
#include "tinyRPC/net/tcp/tcp_acceptor.h"
#include "tinyRPC/net/tcp/tcp_connection.h"
#include "tinyRPC/net/tcp/net_addr.h"
#include "tinyRPC/net/eventloop.h"
#include "tinyRPC/net/io_thread_group.h"

namespace tinyRPC
{
    class TcpServer
    {
    public:
        TcpServer(NetAddr::s_ptr local_addr);
        ~TcpServer();
        void start();

    private:
        void init();
        // 当有新客户端连接之后需要执行的任务
        void onAccept();

    private:
        TcpAcceptor::s_ptr m_acceptor;
        NetAddr::s_ptr m_local_addr;        // 本地监听地址
        EventLoop *m_main_event_loop{NULL}; // mainReactor

        IOThreadGroup *m_io_thread_group{NULL}; // subReactor 组
        FdEvent *m_listen_fd_event;
        int m_client_counts{0};
        std::set<TcpConnection::s_ptr> m_client;
    };
} // tinyRPC

#endif // __TCP_SERVER__