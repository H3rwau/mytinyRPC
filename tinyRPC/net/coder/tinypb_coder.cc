#include <vector>
#include <string.h>
#include <arpa/inet.h>
#include "tinyRPC/common/log.h"
#include "tinyRPC/common/util.h"
#include "tinyRPC/net/coder/tinypb_coder.h"
#include "tinyRPC/net/coder/tinypb_protocol.h"

namespace tinyRPC
{
    // 将 message 对象转化为字节流，写入到 buffer
    void TinyPBCoder::encode(std::vector<AbstractProtocol::s_ptr> &messages, TcpBuffer::s_ptr out_buffer)
    {
        for (auto &protocl_ptr : messages)
        {
            std::shared_ptr<TinyPBProtocol> msg = std::dynamic_pointer_cast<TinyPBProtocol>(protocl_ptr);
            int len = 0;

            // 解析msg，将解析后的内容写入到缓冲区中
            const char *buf = encodeTinyPB(msg, len);
            if (buf && 0 != len)
            {
                out_buffer->writeToBuffer(buf, len);
            }
            if (buf)
            {
                free((void *)buf);
                buf = nullptr;
            }
        }
    }
    // 将 buffer 里面的字节流转换为 message 对象
    void TinyPBCoder::decode(std::vector<AbstractProtocol::s_ptr> &out_messages, TcpBuffer::s_ptr buffer)
    {
        while (1)
        {
            // 遍历buffer,找到PB_START,找到后，解析整个包的长度。
            // 确定结束符的位置，判断结束符是否正确。继续找下个包
            std::vector<char> tmp = buffer->m_buffer;
            int start_index = buffer->readIndex();
            int end_index = -1;
            int pk_len = 0;
            bool parse_success = false;
            int i = 0;
            for (i = start_index; i < buffer->writeIndex(); ++i)
            {
                if (tmp[i] == TinyPBProtocol::PB_START)
                {
                    // 读后面四个字节，网络字节序转为主机字节序
                    if (i + 4 < buffer->writeIndex())
                    {
                        pk_len = getInt32FromNetByte(&tmp[i + 1]);
                        DEBUGLOG("get pk_len = %d", pk_len);

                        int j = i + pk_len - 1;
                        if (j >= buffer->writeIndex())
                        {
                            continue;
                        }
                        if (tmp[j] == TinyPBProtocol::PB_END)
                        {
                            start_index = i;
                            end_index = j;
                            parse_success = true;
                            break;
                        }
                    }
                }
            }
            if (i >= buffer->writeIndex())
            {
                DEBUGLOG("decode end, read all buffer data");
                return;
            }
            if (parse_success)
            {
                buffer->moveReadIndex(end_index - start_index + 1);
                std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();

                message->m_pk_len = pk_len;

                // 找到req_id_len的索引位置
                int req_id_len_index = start_index + sizeof(char) + sizeof(message->m_pk_len);

                if (req_id_len_index >= end_index)
                {
                    message->parse_success = false;
                    ERRORLOG("parse error, req_id_len_index[%d] >= end_index[%d]", req_id_len_index, end_index);
                    continue;
                }
                // 解析req_id_len
                message->m_req_id_len = getInt32FromNetByte(&tmp[req_id_len_index]);
                DEBUGLOG("parse req_id_len=%d", message->m_req_id_len);

                // 找到req_id的索引位置
                int req_id_index = req_id_len_index + sizeof(message->m_req_id_len);
                // 解析req_id
                char req_id[100] = {0};
                memcpy(&req_id[0], &tmp[req_id_index], message->m_req_id_len);
                message->m_req_id = std::string(req_id);
                DEBUGLOG("parse req_id=%s", message->m_req_id.c_str());

                // 找到method_name_len的索引位置
                int method_name_len_index = req_id_index + message->m_req_id_len;
                if (method_name_len_index >= end_index)
                {
                    message->parse_success = false;
                    ERRORLOG("parse error, method_name_len_index[%d] >= end_index[%d]", method_name_len_index, end_index);
                    continue;
                }
                // 解析method_name_len
                message->m_method_name_len = getInt32FromNetByte(&tmp[method_name_len_index]);
                // 找到method_name的索引位置
                int method_name_index = method_name_len_index + sizeof(message->m_method_name_len);
                // 解析method_name
                char method_name[512] = {0};
                memcpy(&method_name[0], &tmp[method_name_index], message->m_method_name_len);
                message->m_method_name = std::string(method_name);
                DEBUGLOG("parse method_name=%s", message->m_method_name.c_str());
                // 找到error_code的索引位置
                int err_code_index = method_name_index + message->m_method_name_len;
                if (err_code_index >= end_index)
                {
                    message->parse_success = false;
                    ERRORLOG("parse error, err_code_index[%d] >= end_index[%d]", err_code_index, end_index);
                    continue;
                }
                // 解析error_code
                message->m_err_code = getInt32FromNetByte(&tmp[err_code_index]);
                // 找到error_info_len的索引位置
                int error_info_len_index = err_code_index + sizeof(message->m_err_code);
                if (error_info_len_index >= end_index)
                {
                    message->parse_success = false;
                    ERRORLOG("parse error, error_info_len_index[%d] >= end_index[%d]", error_info_len_index, end_index);
                    continue;
                }
                // 解析error_info_len
                message->m_err_info_len = getInt32FromNetByte(&tmp[error_info_len_index]);
                // 找到error_info的索引位置
                int err_info_index = error_info_len_index + sizeof(message->m_err_info_len);
                // 解析error_info
                char error_info[512] = {0};
                memcpy(&error_info[0], &tmp[err_info_index], message->m_err_info_len);
                message->m_err_info = std::string(error_info);
                DEBUGLOG("parse error_info=%s", message->m_err_info.c_str());
                // 求出数据内容的长度
                int pb_data_len = message->m_pk_len - message->m_method_name_len - message->m_req_id_len - message->m_err_info_len - 2 - 24;
                // 求出数据内容的开始索引位置
                int pd_data_index = err_info_index + message->m_err_info_len;

                message->m_pb_data = std::string(&tmp[pd_data_index], pb_data_len);
                DEBUGLOG("parse pb_data = %s", message->m_pb_data.c_str());
                // 这里校验和去解析 TODO:
                message->parse_success = true;

                out_messages.push_back(message);
            }
        }
    }

    const char *TinyPBCoder::encodeTinyPB(std::shared_ptr<TinyPBProtocol> message, int &len)
    {

        if (message->m_req_id.empty())
        {
            message->m_req_id = "123456789";
        }

        DEBUGLOG("req_id = %s", message->m_req_id.c_str());
        int pk_len = 2 + 24 + message->m_req_id.length() + message->m_method_name.length() + message->m_err_info.length() + message->m_pb_data.length();
        DEBUGLOG("pk_len = %", pk_len);
        // 开辟一个buf，记得之后free
        char *buf = reinterpret_cast<char *>(malloc(pk_len));
        char *tmp = buf;
        *tmp = TinyPBProtocol::PB_START;
        tmp++;

        // 每个都需要转网络字节序
        int32_t pk_len_net = htonl(pk_len);
        memcpy(tmp, &pk_len_net, sizeof(pk_len_net));
        tmp += sizeof(pk_len_net);
        int req_id_len = message->m_req_id.length();

        int32_t req_id_len_net = htonl(req_id_len);
        memcpy(tmp, &req_id_len_net, sizeof(req_id_len_net));
        tmp += sizeof(req_id_len_net);
        if (!message->m_req_id.empty())
        {
            memcpy(tmp, &(message->m_req_id[0]), req_id_len);
            tmp += req_id_len;
        }

        int method_name_len = message->m_method_name.length();
        int32_t method_name_len_net = htonl(method_name_len);
        memcpy(tmp, &method_name_len_net, sizeof(method_name_len_net));
        tmp += sizeof(method_name_len_net);
        if (!message->m_method_name.empty())
        {
            memcpy(tmp, &(message->m_method_name[0]), method_name_len);
            tmp += method_name_len;
        }

        int32_t err_code_net = htonl(message->m_err_code);
        memcpy(tmp, &err_code_net, sizeof(err_code_net));
        tmp += sizeof(err_code_net);

        int err_info_len = message->m_err_info.length();
        int32_t err_info_len_net = htonl(err_info_len);
        memcpy(tmp, &err_info_len_net, sizeof(err_info_len_net));
        tmp += sizeof(err_info_len_net);
        if (!message->m_err_info.empty())
        {
            memcpy(tmp, &(message->m_err_info[0]), err_info_len);
            tmp += err_info_len;
        }

        if (!message->m_pb_data.empty())
        {
            memcpy(tmp, &(message->m_pb_data[0]), message->m_pb_data.length());
            tmp += message->m_pb_data.length();
        }

        int32_t check_sum_net = htonl(1);
        memcpy(tmp, &check_sum_net, sizeof(check_sum_net));
        tmp += sizeof(check_sum_net);
        *tmp = TinyPBProtocol::PB_END;

        message->m_pk_len = pk_len;
        message->m_req_id_len = req_id_len;
        message->m_method_name_len = method_name_len;
        message->m_err_info_len = err_info_len;
        message->parse_success = true;
        len = pk_len;

        DEBUGLOG("encode message[%s] success", message->m_req_id.c_str());
        return buf;
    }
} // tinyRPC