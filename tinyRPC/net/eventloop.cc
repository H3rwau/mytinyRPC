#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <string.h>
#include "tinyRPC/net/eventloop.h"
#include "tinyRPC/common/log.h"
#include "tinyRPC/common/util.h"

namespace tinyRPC
{
    static thread_local EventLoop *t_current_eventloop = nullptr;
    static int g_epoll_max_timeout = 10000;
    static int g_epoll_max_events = 128;

    EventLoop::EventLoop()
    {
        if (t_current_eventloop != nullptr)
        {
            ERRORLOG("failed to create eventLoop, this thread has created eventLoop");
            exit(0);
        }
        m_thread_id = getThreadId();

        m_epoll_fd = epoll_create(10);

        if (-1 == m_epoll_fd)
        {
            ERRORLOG("failed to create eventLoop, epoll_create error,error info [%d,%s]", errno, strerror(errno));
            exit(0);
        }

        initWakeUpFdEvent();
        initTimer();

        INFOLOG("create eventLoop in thread %d ", m_thread_id);
        t_current_eventloop = this;
    }
    EventLoop::~EventLoop()
    {
        close(m_epoll_fd);
        if (nullptr != m_wakeup_fd_event)
        {
            delete m_wakeup_fd_event;
            m_wakeup_fd_event = nullptr;
        }
        if (m_timer)
        {
            delete m_timer;
            m_timer = nullptr;
        }
    }
    EventLoop *EventLoop::GetCurrentEventLoop()
    {
        if (t_current_eventloop)
        {
            return t_current_eventloop;
        }
        t_current_eventloop = new EventLoop();
        return t_current_eventloop;
    }
    void EventLoop::initTimer()
    {
        m_timer = new Timer();
        addEpollEvent(m_timer);
    }
    void EventLoop::addTimerEvent(TimerEvent::s_ptr event)
    {
        m_timer->addTimerEvent(event);
    }
    void EventLoop::loop()
    {
        while (!m_stop_flag)
        {
            std::unique_lock<std::mutex> ulk(m_mutex);
            std::queue<std::function<void()>> temp_tasks;
            m_pending_tasks.swap(temp_tasks);
            ulk.unlock();

            while (!temp_tasks.empty())
            {
                std::function<void()> cb = temp_tasks.front();
                temp_tasks.pop();
                if (cb)
                {
                    cb();
                }
            }

            // int timeout = g_epoll_max_timeout;
            epoll_event r_events[g_epoll_max_events];

            int rt = epoll_wait(m_epoll_fd, r_events, g_epoll_max_events, g_epoll_max_timeout);
            DEBUGLOG("now end epoll wait, rt =%d", rt);

            if (rt < 0)
            {
                ERRORLOG("epoll_wait error ,error =[%d,%s]", errno, strerror(errno));
            }
            else
            {
                for (int i = 0; i < rt; ++i)
                {
                    epoll_event trigger_event = r_events[i];
                    FdEvent *fd_event = static_cast<FdEvent *>(trigger_event.data.ptr);

                    if (fd_event == nullptr)
                    {
                        ERRORLOG("fd_event = nullptr ,continue");
                        continue;
                    }

                    if (trigger_event.events & EPOLLIN)
                    {
                        DEBUGLOG("fd %d trigger EPOLLIN event", fd_event->getFd());
                        addTask(fd_event->handler(FdEvent::IN_EVENT));
                    }
                    if (trigger_event.events & EPOLLOUT)
                    {
                        DEBUGLOG("fd %d trigger EPOLLOUT event", fd_event->getFd());
                        addTask(fd_event->handler(FdEvent::OUT_EVENT));
                    }
                }
            }
        }
    }
    void EventLoop::wakeup()
    {
        INFOLOG("WAKE UP");
        m_wakeup_fd_event->wakeup();
    }
    void EventLoop::stop()
    {
        m_stop_flag = true;
    }

    void EventLoop::addToEpoll(FdEvent *event)
    {
        auto it = m_listen_fds.find(event->getFd());
        int op = EPOLL_CTL_ADD;
        if (it != m_listen_fds.end())
        {
            op = EPOLL_CTL_MOD;
        }
        epoll_event tmp = event->getEpollEvent();
        INFOLOG("epoll_event.events = %d", (int)tmp.events);

        int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp);
        if (-1 == rt)
        {
            ERRORLOG("failed epoll_ctl when add fd, errno = [%d,%s]", errno, strerror(errno));
        }
        m_listen_fds.insert(event->getFd());
        DEBUGLOG("add event success, fd[%d]", event->getFd());
    }
    void EventLoop::deleteToEpoll(FdEvent *event)
    {
        auto it = m_listen_fds.find(event->getFd());
        if (it == m_listen_fds.end())
        {
            return;
        }
        int op = EPOLL_CTL_DEL;
        epoll_event tmp = event->getEpollEvent();
        INFOLOG("epoll_event.events = %d", (int)tmp.events);

        int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), nullptr);
        if (-1 == rt)
        {
            ERRORLOG("failed epoll_ctl when del fd, errno = [%d,%s]", errno, strerror(errno));
        }
        m_listen_fds.erase(event->getFd());
        DEBUGLOG("delete event success, fd[%d]", event->getFd());
    }
    // 在 addEpollEvent 中判断 isInLoopThread() 是为了确保 addToEpoll 方法在事件循环所在线程中执行，
    // 从而保证线程安全和正确的事件处理顺序。
    // 如果在非事件循环线程中调用 addEpollEvent，则将操作封装为任务，并通过任务队列安全地交给事件循环线程处理

    void EventLoop::addEpollEvent(FdEvent *event)
    {
        DEBUGLOG("into addEpollEvent fd [%d]", event->getFd());
        if (isInLoopThread())
        {
            DEBUGLOG("isInLoopThread, into addEpollEvent fd [%d]", event->getFd());
            addToEpoll(event);
        }
        else
        {
            DEBUGLOG("add Task,into addEpollEvent fd [%d]", event->getFd());
            auto cb = [this, event]()
            {
                addToEpoll(event);
            };
            addTask(cb, true);
        }
    }
    void EventLoop::deleteEpollEvent(FdEvent *event)
    {
        if (isInLoopThread())
        {
            deleteToEpoll(event);
        }
        else
        {
            auto cb = [this, event]()
            {
                deleteToEpoll(event);
            };
            addTask(cb, true);
        }
    }

    bool EventLoop::isInLoopThread()
    {
        return getThreadId() == m_thread_id;
    }

    void EventLoop::addTask(std::function<void()> cb, bool is_wake_up)
    {
        {
            std::unique_lock<std::mutex> ulk(m_mutex);
            m_pending_tasks.push(cb);
        }
        if (is_wake_up)
        {
            wakeup();
        }
    }

    void EventLoop::dealWakeup()
    {
    }

    void EventLoop::initWakeUpFdEvent()
    {
        m_wakeup_fd = eventfd(0, EFD_NONBLOCK);
        if (m_wakeup_fd < 0)
        {
            ERRORLOG("failed to create eventLoop, eventfd create error,error info [%d,%s]", errno, strerror(errno));
            exit(0);
        }
        INFOLOG("wakeup fd = %d", m_wakeup_fd);

        m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);
        m_wakeup_fd_event->listen(FdEvent::IN_EVENT, [this]()
                                  { char buf[8];
                                  while(read(m_wakeup_fd,buf,8)!=-1&&errno!=EAGAIN){
                                      DEBUGLOG("read full bytes from wakeup fd [%d]", m_wakeup_fd);
                                  } });
        addEpollEvent(m_wakeup_fd_event);
    }

} // tinyRPC