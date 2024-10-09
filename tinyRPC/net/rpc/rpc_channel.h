#ifndef __RPC_CHANNEL__
#define __RPC_CHANNEL__
#include <memory>
#include <google/protobuf/service.h>
#include "tinyRPC/net/tcp/net_addr.h"
#include "tinyRPC/net/tcp/tcp_client.h"
#include "tinyRPC/net/timer_event.h"

namespace tinyRPC
{

    #define NEWMESSAGE(type, var_name)                                                                         \
    std::shared_ptr<type> var_name = std::make_shared<type>();

    #define NEWRPCCONTROLLER(var_name)                                                                         \
        std::shared_ptr<tinyRPC::RpcController> var_name = std::make_shared<tinyRPC::RpcController>();

    #define NEWRPCCHANNEL(addr, var_name)                                                                      \
        std::shared_ptr<tinyRPC::RpcChannel>                                                                    \
            var_name = std::make_shared<tinyRPC::RpcChannel>(std::make_shared<tinyRPC::IPV4NetAddr>(addr));

    #define CALLRPRC(method_name, controller, request, response, closure)                                \
    {                                                                                                          \
        channel->Init(controller, request, response, closure);                                                 \
        Order_Stub(channel.get()).method_name(controller.get(), request.get(), response.get(), closure.get()); \
    }

    class RpcChannel : public google::protobuf::RpcChannel,
                       public std::enable_shared_from_this<RpcChannel>
    {
    public:
        using s_ptr = std::shared_ptr<RpcChannel>;
        using controller_s_ptr = std::shared_ptr<google::protobuf::RpcController>;
        using message_s_ptr = std::shared_ptr<google::protobuf::Message>;
        using closure_s_ptr = std::shared_ptr<google::protobuf::Closure>;
        RpcChannel(NetAddr::s_ptr peer_addr);
        ~RpcChannel();
        void Init(controller_s_ptr controller, message_s_ptr req, message_s_ptr res, closure_s_ptr done);
        void CallMethod(const google::protobuf::MethodDescriptor *method,
                        google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                        google::protobuf::Message *response, google::protobuf::Closure *done);

        google::protobuf::RpcController *getController();
        google::protobuf::Message *getRequest();
        google::protobuf::Message *getResponse();
        google::protobuf::Closure *getClosure();
        TcpClient *getTcpClient();
        TimerEvent::s_ptr getTimerEvent();

    private:
        NetAddr::s_ptr m_peer_addr{nullptr};
        NetAddr::s_ptr m_local_addr{nullptr};

        controller_s_ptr m_controller{nullptr};
        message_s_ptr m_request{nullptr};
        message_s_ptr m_response{nullptr};
        closure_s_ptr m_closure{nullptr};
        bool m_is_init{false};
        TcpClient::s_ptr m_client{nullptr};
        TimerEvent::s_ptr m_timer_event{nullptr};
    };
} // tinyRPC

#endif // __RPC_CHANNEL__