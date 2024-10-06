#ifndef __TCP_ACCEPTOR__
#define __TCP_ACCEPTOR__
#include <memory>
#include "tinyRPC/net/tcp/net_addr.h"
namespace tinyRPC
{
    class TcpAcceptor
    {
    public:
        using s_ptr = std::shared_ptr<TcpAcceptor>;
        TcpAcceptor(NetAddr::s_ptr local_addr);
        //~TcpAcceptor();
        std::pair<int,NetAddr::s_ptr> accept();
        int getListenFd();

    private:
        NetAddr::s_ptr m_local_addr; // 服务端监听的地址，addr -> ip:port
        int m_family{-1};
        int m_listenfd{-1}; // 监听套接字
    };
} // tinyRPC

#endif // __TCP_ACCEPTOR__