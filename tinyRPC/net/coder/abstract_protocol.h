#ifndef __ABSTRACT_PROTOCOL__
#define __ABSTRACT_PROTOCOL__

#include <memory>
#include <string>
namespace tinyRPC
{
    class AbstractProtocol 
    :public std::enable_shared_from_this<AbstractProtocol>
    {
    public:
        using s_ptr = std::shared_ptr<AbstractProtocol>;

        virtual ~AbstractProtocol() {}

    public:
        std::string m_req_id;//请求号，唯一标识一个请求
    };
} // tinyRPC

#endif // __ABSTRACT_PROTOCOL__