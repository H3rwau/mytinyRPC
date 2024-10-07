#if !defined(__UTIL_H__)
#define __UTIL_H__
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>

namespace tinyRPC
{
    pid_t getPid();
    pid_t getThreadId();
    int64_t getNowMS(); // 获得现在的毫秒数
    int32_t getInt32FromNetByte(const char *buf);//将网络字节序的int32转为主机字节序
}

#endif // __UTIL_H__
