#ifndef __FD_EVENT__
#define __FD_EVENT__
#include <functional>
#include <sys/epoll.h>

namespace tinyRPC
{
    class FdEvent
    {
    public:
        enum TriggerEvent
        {
            IN_EVENT = EPOLLIN,
            OUT_EVENT = EPOLLOUT,
        };
        FdEvent(int fd);
        FdEvent();
        ~FdEvent();
        void setNonBlock();
        void cancel(TriggerEvent event_type);
        std::function<void()> handler(TriggerEvent event_type);

        void listen(TriggerEvent event_type, std::function<void()> callback);

        int getFd() const
        {
            return m_fd;
        }

        epoll_event getEpollEvent() const
        {
            return m_listen_events;
        }

    protected:
        int m_fd = -1;

        epoll_event m_listen_events;
        std::function<void()> m_read_callback;
        std::function<void()> m_write_callback;
    };
} // tinyRPC
#endif // __FD_EVENT__