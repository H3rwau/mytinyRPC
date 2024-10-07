#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "tinyRPC/common/log.h"
#include "tinyRPC/net/tcp/tcp_client.h"
#include "tinyRPC/net/eventloop.h"
#include "tinyRPC/net/fd_event_group.h"

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
        m_connection = std::make_shared<TcpConnection>(m_event_loop, m_fd, 128, peer_addr);
        m_connection->setConnectionType(TcpConnectionType::TcpConnectionByClient);
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
                    int error = 0;
                    socklen_t error_len = sizeof(error);
                    getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &error, &error_len);
                    if (error == 0) {//错误码返回0代表连接成功
                        DEBUGLOG("errno == 0 TcpClient::connect [%s] success", m_peer_addr->toString().c_str());
                        if (done) {
                            done();
                        }
                    } else {
                        ERRORLOG("TcpClient::connect errror, errno=%d, error=%s", errno, strerror(errno));
                    }
                    // 连接完后需要去掉可写事件的监听，不然会一直触发
                    m_fd_event->cancel(FdEvent::OUT_EVENT);
                    m_event_loop->addEpollEvent(m_fd_event); });

                m_event_loop->addEpollEvent(m_fd_event);
                if (!m_event_loop->isLooping())
                {
                    m_event_loop->loop();
                }
            }
            else
            {
                ERRORLOG("connect errror, errno=%d, error=%s", errno, strerror(errno));
            }
        }
    }
    // 异步的发送 message
    // 如果发送 message 成功，会调用 done 函数， 函数的入参就是 message 对象
    void TcpClient::writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done)
    {
    }
    // 异步的读取 message
    // 如果读取 message 成功，会调用 done 函数， 函数的入参就是 message 对象
    void TcpClient::readMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done)
    {
    }
} // tinyRPC