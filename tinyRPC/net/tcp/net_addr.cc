#include <string.h>
#include "tinyRPC/common/log.h"
#include "tinyRPC/net/tcp/net_addr.h"

namespace tinyRPC
{
    IPV4NetAddr::IPV4NetAddr(const std::string &ip, uint16_t port)
        : m_ip(ip), m_port(port)
    {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
        m_addr.sin_port = htons(m_port);
    }
    IPV4NetAddr::IPV4NetAddr(const std::string &addr)
    {
        size_t i = addr.find_first_of(":");
        if (i == addr.npos)
        {
            ERRORLOG("invalid ipv4 addr %s", addr.c_str());
            return;
        }
        m_ip = addr.substr(0, i);
        m_port = std::atoi(addr.substr(i + 1, addr.size() - i - 1).c_str());
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
        m_addr.sin_port = htons(m_port);
    }
    IPV4NetAddr::IPV4NetAddr(sockaddr_in addr)
        : m_addr(addr)
    {
        m_ip = std::string(inet_ntoa(m_addr.sin_addr));
        m_port = ntohs(m_addr.sin_port);
    }
    sockaddr *IPV4NetAddr::getSockAddr()
    {
        return reinterpret_cast<sockaddr *>(&m_addr);
    }
    socklen_t IPV4NetAddr::getSockLen()
    {
        return sizeof(m_addr);
    }
    int IPV4NetAddr::getFamily()
    {
        return AF_INET;
    }
    std::string IPV4NetAddr::toString()
    {
        std::string re;
        re = m_ip + ":" + std::to_string(m_port);
        return re;
    }
    bool IPV4NetAddr::checkValid()
    {
        if (m_ip.empty())
        {
            return false;
        }
        if (m_port < 0 || m_port > 65536)
        {
            return false;
        }
        if (inet_addr(m_ip.c_str()) == INADDR_NONE)
        {
            return false;
        }
        return true;
    }
} // tinyRPC