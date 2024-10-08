#ifndef __MSG_ID_UTIL__
#define __MSG_ID_UTIL__
#include <string>

namespace tinyRPC
{
    class MsgIDUtil
    {
    public:
        static std::string GenMsgID();
    };
} // tinyRPC

#endif // __MSG_ID_UTIL__