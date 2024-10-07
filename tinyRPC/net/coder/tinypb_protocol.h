#ifndef __TINYPB_PROTOCOL__
#define __TINYPB_PROTOCOL__

#include <string>
#include "tinyRPC/net/coder/abstract_protocol.h"

namespace tinyRPC
{
    class TinyPBProtocol : public AbstractProtocol
    {
    public:
        TinyPBProtocol() {}
        ~TinyPBProtocol() {}

    public:
        static char PB_START;//协议开始字符
        static char PB_END;//协议结束字符

    public:
        int32_t m_pk_len{0};//数据总长度
        int32_t m_req_id_len{0};//请求id字段的长度
        // req_id 继承父类 该字段是一个string
        int32_t m_method_name_len{0};//请求方法名的长度
        std::string m_method_name;//请求方法名
        int32_t m_err_code{0};//错误码
        int32_t m_err_info_len{0};//错误信息长度
        std::string m_err_info;//错误信息
        std::string m_pb_data;//protocol数据字段
        int32_t m_check_sum{0};//检验和
        bool parse_success{false};
    };
} // tinyRPC

#endif // __TINYPB_PROTOCOL__