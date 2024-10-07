#ifndef __TINYPB_CODER__
#define __TINYPB_CODER__

#include "tinyRPC/net/coder/abstract_coder.h"
#include "tinyRPC/net/coder/tinypb_protocol.h"

namespace tinyRPC
{
    class TinyPBCoder : public AbstractCoder
    {
    public:
        TinyPBCoder() {}
        ~TinyPBCoder() {}

        // 将 message 对象转化为字节流，写入到 buffer
        void encode(std::vector<AbstractProtocol::s_ptr> &messages, TcpBuffer::s_ptr out_buffer);
        // 将 buffer 里面的字节流转换为 message 对象
        void decode(std::vector<AbstractProtocol::s_ptr> &out_messages, TcpBuffer::s_ptr buffer);

    private:
        const char *encodeTinyPB(std::shared_ptr<TinyPBProtocol> message, int &len);
    };
} // tinyRPC

#endif // __TINYPB_CODER__