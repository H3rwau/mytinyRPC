#include <assert.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include "tinyRPC/common/log.h"
#include "tinyRPC/net/tcp/net_addr.h"
#include "tinyRPC/net/tcp/tcp_acceptor.h"

namespace tinyRPC
{
    TcpAcceptor::TcpAcceptor(NetAddr::s_ptr local_addr)
        : m_local_addr(local_addr)
    {
        if (!local_addr->checkValid())
        {
            ERRORLOG("invalid local addr %s", local_addr->toString().c_str());
            exit(0);
        }
        m_family = m_local_addr->getFamily();
        // 新建socket套接字
        m_listenfd = socket(m_family, SOCK_STREAM, 0);
        if (m_listenfd < 0)
        {
            ERRORLOG("invalid listenfd %d", m_listenfd);
            exit(0);
        }
        // 重用地址
        int val = 1;
        if (setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0)
        {
            ERRORLOG("setsockopt REUSEADDR error, errno=%d, error=%s", errno, strerror(errno));
        }

        socklen_t len = m_local_addr->getSockLen();
        if (bind(m_listenfd, m_local_addr->getSockAddr(), len) != 0)
        {
            ERRORLOG("bind error, errno=%d, error=%s", errno, strerror(errno));
            exit(0);
        }
        if (listen(m_listenfd, 1000) != 0)
        {
            ERRORLOG("listen error, errno=%d, error=%s", errno, strerror(errno));
            exit(0);
        }
    }

    int TcpAcceptor::getListenFd()
    {
        return m_listenfd;
    }
    std::pair<int, NetAddr::s_ptr> TcpAcceptor::accept()
    {
        if (m_family == AF_INET)
        {
            sockaddr_in client_addr;
            memset(&client_addr, 0, sizeof(client_addr));
            socklen_t clien_addr_len = sizeof(clien_addr_len);

            int client_fd = ::accept(m_listenfd, reinterpret_cast<sockaddr *>(&client_addr), &clien_addr_len);
            if (client_fd < 0)
            {
                ERRORLOG("accept error, errno=%d, error=%s", errno, strerror(errno));
            }
            //IPV4NetAddr peer_addr(client_addr);
            IPV4NetAddr::s_ptr peer_addr = std::make_shared<IPV4NetAddr>(client_addr);
            INFOLOG("A client have accpeted succ, peer addr [%s]", peer_addr->toString().c_str());
            return std::make_pair(client_fd,peer_addr);
        }
        else
        {
            // ...
            return std::make_pair(-1,nullptr);
        }
    }
} // tinyRPC