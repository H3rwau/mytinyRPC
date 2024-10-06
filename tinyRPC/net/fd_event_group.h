#ifndef __FD_EVENT_GROUP__
#define __FD_EVENT_GROUP__

#include <vector>
#include <mutex>
#include "tinyRPC/net/fd_event.h"


namespace  tinyRPC{
    class FdEventGroup
    {
    public:
        FdEventGroup(int size);
        ~FdEventGroup();
        FdEvent *getFdEvent(int fd);

    public:
        static FdEventGroup *GetFdEventGroup();

    private:
        int m_size{0};
        std::vector<FdEvent *> m_fd_group;
        std::mutex m_mutex;
    };
}//tinyRPC

#endif // __FD_EVENT_GROUP__