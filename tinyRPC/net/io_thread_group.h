#ifndef __IO_THREAD_GROUP__
#define __IO_THREAD_GROUP__
#include <vector>
#include "tinyRPC/common/log.h"
#include "tinyRPC/net/io_thread.h"
namespace tinyRPC
{
    class IOThreadGroup
    {
    public:
        IOThreadGroup(int size);
        ~IOThreadGroup();

        void start();
        void stop();

        IOThread *getIOThread();

    private:
        int m_size{0};
        int m_index{0};
        std::vector<IOThread *> m_io_thread_groups;
    };

} // tinyRPC

#endif // __IO_THREAD_GROUP__