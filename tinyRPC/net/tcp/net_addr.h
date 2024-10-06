#ifndef __NET_ADDR__
#define __NET_ADDR__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>

namespace tinyRPC
{
    class NetAddr
    {
    public:
        typedef std::shared_ptr<NetAddr> s_ptr;
        virtual sockaddr *getSockAddr() = 0;
        virtual socklen_t getSockLen() = 0;
        virtual int getFamily() = 0;
        virtual std::string toString() = 0;
        virtual bool checkValid() = 0;

    private:
    };
    class IPV4NetAddr
        : public NetAddr
    {
    public:
        IPV4NetAddr(const std::string &ip, uint16_t port);
        IPV4NetAddr(const std::string &addr);
        IPV4NetAddr(sockaddr_in addr);
        sockaddr *getSockAddr();
        socklen_t getSockLen();
        int getFamily();
        std::string toString();
        bool checkValid();

    private:
        std::string m_ip;
        uint16_t m_port{0};
        sockaddr_in m_addr;
    };
} // tinyRPC

#endif // __NET_ADDR__