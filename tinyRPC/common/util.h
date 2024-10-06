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
}

#endif // __UTIL_H__
