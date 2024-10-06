#include "tinyRPC/common/util.h"

namespace tinyRPC
{
    static int g_pid = 0;
    static thread_local int g_thread_pid = 0;
    pid_t getPid()
    {
        if (g_pid != 0)
        {
            return g_pid;
        }
        g_pid = getpid();
        return g_pid;
    }
    pid_t getThreadId()
    {
        if (g_thread_pid != 0)
        {
            return g_thread_pid;
        }
        g_thread_pid = syscall(SYS_gettid);
        return g_thread_pid;
    }
    int64_t getNowMS() // 获得现在的毫秒数
    {
        timeval tval;
        gettimeofday(&tval, nullptr);
        return tval.tv_sec * 1000 + tval.tv_usec / 1000;
    }
}
