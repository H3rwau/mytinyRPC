#ifndef __STRING_CODER__
#define __STRING_CODER__
#include "tinyRPC/net/coder/abstract_protocol.h"
#include "tinyRPC/net/coder/abstract_coder.h"

namespace tinyRPC
{
    class StringProtocol
        : public AbstractProtocol
    {
    public:
        std::string info;
    };

    class StringCoder
        : public AbstractCoder
    {
    public:
        // 将 message 对象转化为字节流，写入到 buffer
        void encode(std::vector<AbstractProtocol::s_ptr> &messages, TcpBuffer::s_ptr out_buffer)
        {
            for (size_t i = 0; i < messages.size(); ++i)
            {
                std::shared_ptr<StringProtocol> msg = std::dynamic_pointer_cast<StringProtocol>(messages[i]);
                out_buffer->writeToBuffer(msg->info.c_str(), msg->info.length());
            }
        }
        // 将 buffer 里面的字节流转换为 message 对象
        void decode(std::vector<AbstractProtocol::s_ptr> &out_messages, TcpBuffer::s_ptr buffer)
        {
            std::vector<char> re;
            buffer->readFromBuffer(re, buffer->readAble());
            std::string info;
            for (size_t i = 0; i < re.size(); ++i)
            {
                info += re[i];
            }
            // 新建一个msg对象
            std::shared_ptr<StringProtocol> msg = std::make_shared<StringProtocol>();
            msg->info = info;
            msg->m_req_id= "123456";
            out_messages.push_back(msg);
        }
    };

} // tinyRPC

#endif // __STRING_CODER__