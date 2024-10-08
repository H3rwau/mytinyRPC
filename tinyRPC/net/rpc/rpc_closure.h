#ifndef __RPC_CLOSURE__
#define __RPC_CLOSURE__

#include <google/protobuf/stubs/callback.h>
#include <functional>

namespace  tinyRPC{
    class RpcClosure:public google::protobuf::Closure 
    {
    public:
        void Run() override{
            if(m_cb){
                m_cb();
            }
        }
    
    private:
        std::function<void()> m_cb{nullptr};
    };
}//tinyRPC

#endif // __RPC_CLOSURE__