#include <unistd.h>
#include "tinyRPC/common/log.h"
#include "tinyRPC/net/wakeup_fd_event.h"
namespace tinyRPC
{
    WakeUpFdEvent::WakeUpFdEvent(int fd)
        : FdEvent(fd)
    {
    }
    WakeUpFdEvent::~WakeUpFdEvent()
    {
    }

    void WakeUpFdEvent::wakeup()
    {
        char buf[8] = {'a'};
        int rt = write(m_fd, buf, sizeof(buf));
        if (rt != 8)
        {
            ERRORLOG("write to wakeup fd less than 8 bytes, fd[%d]", m_fd);
        }
        DEBUGLOG("success read 8 bytes");
    }
} // tinyRPC
