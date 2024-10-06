#ifndef __TIMER__
#define __TIMER__

#include <map>
#include <mutex>
#include "tinyRPC/net/fd_event.h"
#include "tinyRPC/net/timer_event.h"

namespace tinyRPC
{
    // Timer继承FdEvent,读事件就绪就执行Timer中的所有需要执行的定时任务，也就是执行每一个timer_event的回调函数
    class Timer
        : public FdEvent
    {
    public:
        Timer();
        ~Timer();
        void addTimerEvent(TimerEvent::s_ptr event);
        void deleteTimerEvent(TimerEvent::s_ptr event);
        void onTimer(); // 当发生了IO事件后，eventLoop会执行这个回调函数

        Timer(const Timer &other) = delete;
        Timer &operator=(const Timer &other) = delete;

    private:
        void resetArriveTime();

    private:
        std::multimap<int64_t, TimerEvent::s_ptr> m_pending_events; // 存待执行的事件,key为时间毫秒数
        std::mutex m_mutex;
    };
} // tinyRPC
#endif // __TIMER__