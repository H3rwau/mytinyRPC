#ifndef __IO_THREAD__
#define __IO_THREAD__

#include <pthread.h>
#include <semaphore.h>
#include "tinyRPC/net/eventloop.h"

namespace tinyRPC
{
    class IOThread
    {
    public:
        IOThread();
        ~IOThread();

        EventLoop *getEventLoop();
        void start();
        void join();

    public:
        static void *Main(void *arg);

    private:
        pid_t m_thread_id{-1}; // 线程号
        pthread_t m_thread{0}; // 线程句柄

        EventLoop *m_event_loop{nullptr};

        sem_t m_init_semaphore;
        sem_t m_start_semaphore;
    };
} // tinyRPC

#endif // __IO_THREAD__