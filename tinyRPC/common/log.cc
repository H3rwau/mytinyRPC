#include "tinyRPC/common/log.h"
#include "tinyRPC/common/util.h"
#include "tinyRPC/common/config.h"
namespace tinyRPC
{
    Loglevel Logger::m_log_level;
    std::string LoglevelToString(Loglevel level)
    {
        switch (level)
        {
        case Debug:
            return "DEBUG";
        case Info:
            return "INFO";
        case Error:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }
    Loglevel StringToLoglevel(std::string str)
    {
        if (str == "DEBUG")
        {
            return Debug;
        }
        else if (str == "ERROR")
        {
            return Error;
        }
        else if (str == "INFO")
        {
            return Info;
        }
        else
        {
            return Unknown;
        }
    }
    std::string LogEvent::toString()
    {
        struct timeval now_time;
        gettimeofday(&now_time, nullptr);

        struct tm now_time_t;
        localtime_r(&(now_time.tv_sec), &now_time_t);

        char buf[128];
        strftime(&buf[0], 128, "%y-%m-%d %H:%M:%S", &now_time_t);

        std::string time_str(buf);
        int ms = now_time.tv_usec / 1000;
        time_str += "." + std::to_string(ms);

        m_pid = getPid();
        m_thread_id = getThreadId();
        std::stringstream ss;
        ss << "[" << LoglevelToString(m_level) << "] "
           << "[" << time_str << "] "
           << "[" << m_pid << "," << m_thread_id << "] ";
        return ss.str();
    }
    void Logger::setLogLevel(std::string log_level_str)
    {
        m_log_level = StringToLoglevel(log_level_str);
    }
    void Logger::pushLog(const std::string &msg)
    {
        std::unique_lock<std::mutex> ulk(mutex);
        m_buffer.push(msg);
    }

    void Logger::log()
    {
        std::queue<std::string> temp;
        {
            std::unique_lock<std::mutex> ulk(mutex);
            temp.swap(m_buffer);
        }
        while (!temp.empty())
        {
            std::string msg = temp.front();
            temp.pop();
            printf(msg.c_str());
        }
    }
}