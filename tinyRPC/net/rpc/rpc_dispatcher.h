#ifndef __RPC_DISPATCHER__
#define __RPC_DISPATCHER__

#include <map>
#include <memory>
#include <google/protobuf/service.h>

#include "tinyRPC/net/coder/abstract_coder.h"
#include "tinyRPC/net/coder/tinypb_protocol.h"

namespace  tinyRPC{
    class TcpConnection;

    class RpcDispatcher 
    {
    public:
        using service_s_ptr = std::shared_ptr<google::protobuf::Service>;
        static RpcDispatcher *GetRpcDispatcher();
        void dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response, TcpConnection *connection);
        void registerService(service_s_ptr service);
        void setTinyPBError(std::shared_ptr<TinyPBProtocol> msg, int32_t err_code, const std::string err_info);

    private:
        bool parseServiceFullName(const std::string &full_name, std::string &service_name, std::string &method_name);

    private:
        std::map<std::string, service_s_ptr> m_service_map;
    };
} // tinyRPC

#endif // __RPC_DISPATCHER__