#include <sys/timerfd.h>
#include <string.h>
#include "tinyRPC/net/timer.h"
#include "tinyRPC/common/log.h"
#include "tinyRPC/common/util.h"

namespace tinyRPC
{
    Timer::Timer() : FdEvent()
    {
        m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        // CLOCK_MONOTONIC: 这表示定时器使用的是单调时钟。单调时钟是指从某个特定时间点开始一直向前的时钟，不受系统时间修改的影响。因此，它适合用于测量时间间隔
        // TFD_NONBLOCK: 这个标志使得定时器文件描述符以非阻塞模式工作。在非阻塞模式下，读取操作不会阻塞，如果没有数据可读，会立即返回。
        // TFD_CLOEXEC: 这个标志表示在使用 exec 系列函数执行新程序时，关闭该文件描述符。这样可以防止文件描述符泄漏到新执行的程序中。
        DEBUGLOG("timer fd = %d", m_fd);
        // 绑定回调函数到事件上
        listen(FdEvent::IN_EVENT, std::bind(&Timer::onTimer, this));
    }
    Timer::~Timer()
    {
    }
    void Timer::addTimerEvent(TimerEvent::s_ptr event)
    {
        bool is_reset_timerfd = false;

        std::unique_lock<std::mutex> ulk(m_mutex);
        if (m_pending_events.empty())
        {
            is_reset_timerfd = true;
        }
        else
        {
            auto it = m_pending_events.begin();
            if (it->second->getArriveTime() > event->getArriveTime())
            { // 如果待处理事件中的时间比这个事件的时间要晚
                // 那么需要修改定时器下次执行的时间
                is_reset_timerfd = true;
            }
        }
        m_pending_events.emplace(event->getArriveTime(), event);
        ulk.unlock();
        // 需要修改定时器下次执行的时间
        if (is_reset_timerfd)
        {
            resetArriveTime();
        }
    }
    void Timer::deleteTimerEvent(TimerEvent::s_ptr event)
    {
        event->setCancled(true);
        std::unique_lock<std::mutex> ulk(m_mutex);
        auto begin = m_pending_events.lower_bound(event->getArriveTime());
        auto end = m_pending_events.upper_bound(event->getArriveTime());
        // begin和end之间的事件的时间是和event的时间相等的

        auto it = begin;
        for (; it != end; ++it)
        {
            if (it->second == event)
            {
                break;
            }
        }
        if (it != end)
        {
            m_pending_events.erase(it);
        }
        ulk.unlock();
        DEBUGLOG("success delete TimerEvent at arrive time %lld", event->getArriveTime());
    }
    void Timer::onTimer() // 当发生了IO事件后，eventLoop会执行这个回调函数
    {
        DEBUGLOG("ontimer");
        // timerfd可读需要读一个8字节的数
        char buf[8];
        // 由于设置非阻塞使用while循环读
        while (1)
        {
            // 如果没数据可读,read返回-1并且设置errno为EAGAIN
            if ((read(m_fd, buf, sizeof(buf)) == -1) && errno == EAGAIN)
            {
                break;
            }
        }
        // 接下来执行定时任务
        int64_t now = getNowMS();
        std::vector<TimerEvent::s_ptr> tmps;
        std::vector<std::pair<int64_t, std::function<void()>>> tasks_vector;
        // 由于是任务队列是共享资源,需要加锁
        std::unique_lock<std::mutex> ulk(m_mutex);
        auto it = m_pending_events.begin();
        for (; it != m_pending_events.end(); ++it)
        {
            if (it->first <= now)
            { // 这个事件已经延期了
                if (!it->second->isCancled())
                {
                    // 如果这个事件没有被取消
                    tmps.emplace_back(it->second);
                    tasks_vector.emplace_back(std::make_pair(it->second->getArriveTime(), it->second->getCallBack()));
                }
            }
            else
            {
                break; // 由于m_pending_events是顺序容器所以直接break
            }
        }
        m_pending_events.erase(m_pending_events.begin(), it);
        ulk.unlock();

        // 将需要重复执行的event，再次添加进去
        for (auto it2 = tmps.begin(); it2 != tmps.end(); ++it2)
        {
            if ((*it2)->isRepeated())
            {
                (*it2)->resetArriveTime(); // 加个间隔时间
                addTimerEvent(*it2);
            }
        }

        resetArriveTime();

        for (auto cb : tasks_vector)
        {
            if (cb.second)
            {
                cb.second();
            }
        }
    }

    void Timer::resetArriveTime()
    {
        std::unique_lock<std::mutex> ulk(m_mutex);
        auto tmp = m_pending_events;
        ulk.unlock();

        if (0 == tmp.size())
        {
            return;
        }

        int64_t now = getNowMS();
        auto it = tmp.begin();
        int64_t interval = 0;
        if (it->second->getArriveTime() > now)
        {
            // 使下次的事件到达规定时间就执行
            interval = it->second->getArriveTime() - now;
        }
        else
        { // 事件还没有执行，需要马上执行
            interval = 100;
        }

        // 设置timerfd的间隔时间
        timespec ts;
        memset(&ts, 0, sizeof(ts));
        ts.tv_sec = interval / 1000;              // 将当前的毫秒转换为秒
        ts.tv_nsec = (interval % 1000) * 1000000; // 剩余的毫秒转换为纳秒

        itimerspec value;
        memset(&value, 0, sizeof(value));
        value.it_value = ts;
        // 下次执行需要过多少秒
        int ret = timerfd_settime(m_fd, 0, &value, nullptr);
        if (0 != ret)
        {
            ERRORLOG("timerfd_settime error, errno = %d,error = %s", errno, strerror(errno));
        }
        DEBUGLOG("timer reset to %lld", now + interval);
    }

} // tinyRPC