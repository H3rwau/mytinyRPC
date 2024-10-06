#include "tinyRPC/common/log.h"
#include "tinyRPC/net/io_thread_group.h"

namespace tinyRPC
{
    IOThreadGroup::IOThreadGroup(int size) : m_size(size)
    {
        m_io_thread_groups.resize(size);
        for (int i = 0; i < size; ++i)
        {
            m_io_thread_groups[i] = new IOThread();
        }
    }
    IOThreadGroup::~IOThreadGroup() {}

    void IOThreadGroup::start()
    {
        for (int i = 0; (std::size_t)i < m_io_thread_groups.size(); ++i)
        {
            m_io_thread_groups[i]->start();
        }
    }
    void IOThreadGroup::stop()
    {
        for (int i = 0; (std::size_t)i < m_io_thread_groups.size(); ++i)
        {
            m_io_thread_groups[i]->join();
        }
    }

    IOThread *IOThreadGroup::getIOThread()
    {
        if ((std::size_t)m_index == m_io_thread_groups.size() || -1 == m_index)
        {
            m_index = 0;
        }
        return m_io_thread_groups[m_index++];
    }

} // tinyRPC