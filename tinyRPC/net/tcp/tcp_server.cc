#include "tinyRPC/net/tcp/tcp_server.h"
#include "tinyRPC/net/eventloop.h"
#include "tinyRPC/common/log.h"

namespace tinyRPC
{
    TcpServer::TcpServer(NetAddr::s_ptr local_addr)
        : m_local_addr(local_addr)
    {
        init();
        INFOLOG("TcpServer listen success on [%s]", m_local_addr->toString().c_str());
    }
    TcpServer::~TcpServer()
    {
        if (m_main_event_loop)
        {
            delete m_main_event_loop;
            m_main_event_loop = NULL;
        }
    }
    void TcpServer::start()
    {
        m_io_thread_group->start();
        m_main_event_loop->loop();
    }

    void TcpServer::init()
    {
        m_acceptor = std::make_shared<TcpAcceptor>(m_local_addr);

        m_main_event_loop = EventLoop::GetCurrentEventLoop();
        m_io_thread_group = new IOThreadGroup(2);

        // 将socket套接字建立事件对象
        m_listen_fd_event = new FdEvent(m_acceptor->getListenFd());
        // 将socket套接字的读事件绑定回调函数
        m_listen_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpServer::onAccept, this));
        // 将事件加入到epoll中
        m_main_event_loop->addEpollEvent(m_listen_fd_event);
    }
    // 当有新客户端连接之后需要执行的任务
    void TcpServer::onAccept()
    {

        auto re = m_acceptor->accept();
        int client_fd = re.first;
        NetAddr::s_ptr peer_addr = re.second;
        ++m_client_counts;

        // 把 cleintfd 添加到任意 IO 线程里面
        IOThread *io_thread = m_io_thread_group->getIOThread();
        TcpConnection::s_ptr connection = std::make_shared<TcpConnection>(io_thread->getEventLoop(), client_fd, 128, peer_addr,m_local_addr);
        connection->setState(Connected);
        m_client.insert(connection);
        // m_io_thread_group->getIOThread()->getEventLoop()->addEpollEvent(client_fd_event);
        INFOLOG("TcpServer succ get client, fd=%d", client_fd);
    }
} // tinyRPC