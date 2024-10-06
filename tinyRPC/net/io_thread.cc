#include <pthread.h>
#include <assert.h>
#include "tinyRPC/net/io_thread.h"
#include "tinyRPC/common/log.h"
#include "tinyRPC/common/util.h"

namespace tinyRPC
{
    IOThread::IOThread()
    {
        int rt = sem_init(&m_init_semaphore, 0, 0);
        assert(0 == rt);
        rt = sem_init(&m_start_semaphore, 0, 0);
        assert(0 == rt);
        pthread_create(&m_thread, nullptr, &IOThread::Main, this);

        // wait，直到新线程执行完Main函数的前置
        sem_wait(&m_init_semaphore);
        DEBUGLOG("IOThread [%d] create success", m_thread_id);
    }
    IOThread::~IOThread()
    {
        m_event_loop->stop();
        sem_destroy(&m_init_semaphore);
        sem_destroy(&m_start_semaphore);

        pthread_join(m_thread, nullptr);

        if (m_event_loop)
        {
            delete m_event_loop;
            m_event_loop = nullptr;
        }
    }

    EventLoop *IOThread::getEventLoop()
    {
        return m_event_loop;
    }
    void IOThread::start()
    {
        DEBUGLOG("Now invoke IOthread %d ", m_thread_id);
        sem_post(&m_start_semaphore);
    }
    void IOThread::join()
    {
        pthread_join(m_thread, nullptr);
    }

    void *IOThread::Main(void *arg)
    {
        IOThread *thread = static_cast<IOThread *>(arg);
        thread->m_event_loop = new EventLoop();
        thread->m_thread_id = getThreadId();

        // 唤醒等待的线程
        sem_post(&thread->m_init_semaphore);
        // 让IO线程等待，直到我们主动启动
        DEBUGLOG("IOthread %d created,wait start semaphore", thread->m_thread_id);

        sem_wait(&thread->m_start_semaphore);
        DEBUGLOG("IOThread %d start loop", thread->m_thread_id);

        thread->m_event_loop->loop();
        DEBUGLOG("IOThread %d end loop", thread->m_thread_id);
        return nullptr;
    }
} // tinyRPC