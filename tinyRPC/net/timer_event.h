#ifndef __TIMER_EVENT__
#define __TIMER_EVENT__
#include <functional>
#include <memory>

namespace tinyRPC
{
    class TimerEvent
    {
    public:
        using s_ptr = std::shared_ptr<TimerEvent>;
        TimerEvent(uint64_t interval_time, bool is_repeated, std::function<void()> cb);
        int64_t getArriveTime() const
        {
            return m_arrive_time;
        }
        void setCancled(bool value)
        {
            m_is_canceled = value;
        }
        bool isCancled()
        {
            return m_is_canceled;
        }
        bool isRepeated()
        {
            return m_is_repeated;
        }
        std::function<void()> getCallBack()
        {
            return m_task;
        }
        void resetArriveTime();
        TimerEvent(const TimerEvent &other) = delete;
        TimerEvent &operator=(const TimerEvent &other) = delete;

    private:
        int64_t m_arrive_time; // ms
        int64_t m_interval;    // ms
        bool m_is_repeated{false};
        bool m_is_canceled{false};

        std::function<void()> m_task; // 定时任务执行的函数
    };
} // tinyRPC

#endif // __TIMER_EVENT__