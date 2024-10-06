#ifndef __WAKEUP_FD_EVENT__
#define __WAKEUP_FD_EVENT__
#include "tinyRPC/net/fd_event.h"
namespace tinyRPC
{
    class WakeUpFdEvent
        : public FdEvent
    {
    public:
        WakeUpFdEvent(int fd);
        ~WakeUpFdEvent();

        void wakeup();

    private:
    };
} // tinyRPC

#endif // __WAKEUP_FD_EVENT__