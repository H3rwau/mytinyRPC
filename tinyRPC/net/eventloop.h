#ifndef __EVENTLOOP__
#define __EVENTLOOP__
#include <pthread.h>
#include <set>
#include <functional>
#include <queue>
#include <mutex>

#include "tinyRPC/net/fd_event.h"
#include "tinyRPC/net/wakeup_fd_event.h"
#include "tinyRPC/net/timer.h"

namespace tinyRPC
{
    class EventLoop
    {
    public:
        EventLoop();
        ~EventLoop();

        void loop();
        void wakeup();
        void stop();
        void addEpollEvent(FdEvent *event);
        void deleteEpollEvent(FdEvent *event);
        void addToEpoll(FdEvent *event);
        void deleteToEpoll(FdEvent *event);
        bool isInLoopThread();

        void addTimerEvent(TimerEvent::s_ptr event);
        void initTimer();
        static EventLoop *GetCurrentEventLoop();
        void addTask(std::function<void()> cb, bool is_wake_up = false);

    private:
        void dealWakeup();
        void initWakeUpFdEvent();

    private:
        pid_t m_thread_id = 0;
        int m_epoll_fd = 0;
        int m_wakeup_fd = 0;

        WakeUpFdEvent *m_wakeup_fd_event = nullptr;

        bool m_stop_flag = false;
        std::set<int> m_listen_fds;

        Timer *m_timer{nullptr};

        std::queue<std::function<void()>> m_pending_tasks;
        std::mutex m_mutex;
    };

} // tinyRPC

#endif // __EVENTLOOP__