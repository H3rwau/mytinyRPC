#include "tinyRPC/net/timer_event.h"
#include "tinyRPC/common/log.h"
#include "tinyRPC/common/util.h"

namespace tinyRPC
{

    TimerEvent::TimerEvent(uint64_t interval_time, bool is_repeated, std::function<void()> cb)
        : m_interval(interval_time), m_is_repeated(is_repeated), m_task(cb)
    {
        resetArriveTime();
    }
    void TimerEvent::resetArriveTime()
    {
        m_arrive_time = getNowMS() + m_interval;
        DEBUGLOG("success create timer event, will excute at [%lld]", m_arrive_time);
    }
} // tinyRPC