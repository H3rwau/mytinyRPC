#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "tinyRPC/common/log.h"
#include "tinyRPC/net/tcp/tcp_client.h"
#include "tinyRPC/net/eventloop.h"
#include "tinyRPC/net/fd_event_group.h"
#include "tinyRPC/common/error_code.h"
#include "tinyRPC/net/tcp/net_addr.h"

namespace tinyRPC
{
    TcpClient::TcpClient(NetAddr::s_ptr peer_addr) : m_peer_addr(peer_addr)
    {
        m_event_loop = EventLoop::GetCurrentEventLoop();
        m_fd = socket(peer_addr->getFamily(), SOCK_STREAM, 0);
        if (m_fd < 0)
        {
            ERRORLOG("TcpClient::TcpClient() error, failed to create fd");
            return;
        }
        // 设置了非阻塞的socketfd
        m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);
        m_fd_event->setNonBlock();
        // 建立连接
        m_connection = std::make_shared<TcpConnection>(m_event_loop, m_fd, 128, peer_addr, nullptr, TcpConnectionByClient);
        // m_connection->setConnectionType(TcpConnectionType::TcpConnectionByClient);
    }
    TcpClient::~TcpClient()
    {
        DEBUGLOG("TcpClient::~TcpClient()");
        if (m_fd > 0)
        {
            close(m_fd);
        }
    }
    // 异步的进行 conenct
    // 如果connect 成功，done 会被执行
    /*
    ①当我们将sock设置为非阻塞之后，使用connect去连接服务端，即使服务端开启了，connect系统调用也不会连接成功，connect而是以失败告终，并返回错误
    ②但是非阻塞connect返回的错误是有讲究的：
        如果非阻塞connect返回的错误是EINPROGRESS，代表不是connect系统调用出错了，而是connect可能会在后面才会建立完整地连接(只是当前连接还没有建立完整)，所以我们可以在通过给select、pol或epoll设置等待时间，来等待这个connect的连接成功，从而进一步处理
        如果非阻塞connect返回的错误不是EINPROGRESS，代表就是connect系统调用本身出错了，那么就可以做一些相应的错误处理了
    ③当非阻塞connect以EINPROGRESS错误返回之后，我们可以给select、pol或epoll设置等待时间，并将客户端封装在等待可写的结构中，进一步来等待非阻塞connect客户端与服  务端建立完整地连接，在等待的过程中，如果非阻塞connect建立成功了，客户端的sock_fd就会变成可写的
    ④当非阻塞connect建立成功之后还可以利用getsockopt来读取错误码并清除该socket上的错误：
        如果错误码为0，表示连接成功建立
        否则连接失败
    connect 函数的调用涉及到TCP连接的三次握手过程，通常阻塞的connect 函数会等待三次握手成功或失败后返回，0成功，-1失败。如果对方未响应，要隔6s，重发尝试，可能要等待75s的尝试并最终返回超时，才得知连接失败。即使是一次尝试成功，也会等待几毫秒到几秒的时间，如果此期间有其他事务要处理，则会白白浪费时间，而用非阻塞的connect 则可以做到并行，提高效率。

    而通常，非阻塞的connect 函数与 select 函数配合使用：在一个TCP套接口被设置为非阻塞之后调用connect，connect （函数本身返回-1）会立即返回EINPROGRESS或EWOULDBLOCK错误，表示连接操作正在进行中，但是仍未完成；同时TCP的三路握手操作继续进行；在这之后，我们可以调用select来检查这个链接是否建立成功。

    若建立连接成功，select的结果中该描述符可写；若失败，则可写可读，此时可以使用getsockopt获取错误信息。
*/
    void TcpClient::connect(std::function<void()> done)
    {
        int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
        if (rt == 0)
        {
            DEBUGLOG("rt == 0  TcpClient::connect [%s] success", m_peer_addr->toString().c_str());
            m_connection->setState(Connected);
            initLocalAddr();
            if (done)
            {
                done();
            }
        }
        else if (rt == -1)
        {
            if (errno == EINPROGRESS) // 这时候代表建立连接正在进行
            {
                // epoll 监听可写事件，然后判断错误码
                m_fd_event->listen(FdEvent::OUT_EVENT, [this, done]()
                                   {
                                        int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
                                        if ((rt < 0 && errno == EISCONN) || (rt == 0))
                                        {
                                            DEBUGLOG("connect [%s] sussess", m_peer_addr->toString().c_str());
                                            initLocalAddr();
                                            m_connection->setState(Connected);
                                        }
                                        else
                                        {
                                            if (errno == ECONNREFUSED)
                                            {
                                                m_connect_error_code = ERROR_PEER_CLOSED;
                                                m_connect_error_info = "connect refused, sys error = " + std::string(strerror(errno));
                                            }
                                            else
                                            {
                                                m_connect_error_code = ERROR_FAILED_CONNECT;
                                                m_connect_error_info = "connect unkonwn error, sys error = " + std::string(strerror(errno));
                                            }
                                            ERRORLOG("connect errror, errno=%d, error=%s", errno, strerror(errno));
                                            close(m_fd);
                                            m_fd = socket(m_peer_addr->getFamily(), SOCK_STREAM, 0);
                                        }

                                       // 连接完后需要去掉可写事件的监听，不然会一直触发
                                       //m_fd_event->cancel(FdEvent::OUT_EVENT);
                                       //m_event_loop->addEpollEvent(m_fd_event);
                                        m_event_loop->deleteEpollEvent(m_fd_event);
                                        // 如果连接完成，才会执行回调函数
                                        if (done)
                                        {
                                            done();
                                       } });

                m_event_loop->addEpollEvent(m_fd_event);
                if (!m_event_loop->isLooping())
                {
                    m_event_loop->loop();
                }
            }
            else
            {
                ERRORLOG("connect errror, errno=%d, error=%s", errno, strerror(errno));
                m_connect_error_code = ERROR_FAILED_CONNECT;
                m_connect_error_info = "connect error, sys error = " + std::string(strerror(errno));
                if (done)
                {
                    done();
                }
            }
        }
    }
    // 异步的发送 message
    // 如果发送 message 成功，会调用 done 函数， 函数的入参就是 message 对象
    void TcpClient::writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done)
    {
        // 1. 把 message 对象写入到 Connection 的 buffer, done 也写入
        // 2. 启动 connection 可写事件
        m_connection->pushSendMessage(message, done);
        m_connection->listenWrite();
    }
    // 异步的读取 message
    // 如果读取 message 成功，会调用 done 函数， 函数的入参就是 message 对象
    void TcpClient::readMessage(const std::string &msg_id, std::function<void(AbstractProtocol::s_ptr)> done)
    {
        // 1. 监听可读事件
        // 2. 从 buffer 里 decode 得到 message 对象, 判断是否 msg_id 相等，相等则读成功，执行其回调
        m_connection->pushReadMessage(msg_id, done);
        m_connection->listenRead();
    }
    void TcpClient::stop()
    {
        if (m_event_loop->isLooping())
        {
            m_event_loop->stop();
        }
    }

    int TcpClient::getConnectErrorCode()
    {
        return m_connect_error_code;
    }
    std::string TcpClient::getConnectErrorInfo()
    {
        return m_connect_error_info;
    }
    NetAddr::s_ptr TcpClient::getPeerAddr()
    {
        return m_peer_addr;
    }
    NetAddr::s_ptr TcpClient::getLocalAddr()
    {
        return m_local_addr;
    }
    void TcpClient::initLocalAddr()
    {
        sockaddr_in local_addr;
        socklen_t len = sizeof(local_addr);
        int ret = getsockname(m_fd, reinterpret_cast<sockaddr *>(&local_addr), &len);
        if (ret != 0)
        {
            ERRORLOG("initLocalAddr error, getsockname error. errno=%d, error=%s", errno, strerror(errno));
            return;
        }
        m_local_addr = std::make_shared<IPV4NetAddr>(local_addr);
    }
    void TcpClient::addTimerEvent(TimerEvent::s_ptr timer_event)
    {
        m_event_loop->addTimerEvent(timer_event);
    }

} // tinyRPC