#include <assert.h>
#include <functional>
#include "tinyRPC/common/log.h"
#include "tinyRPC/common/util.h"
#include "tinyRPC/common/config.h"
#include "tinyRPC/net/eventloop.h"
#include "tinyRPC/common/run_time.h"


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

        // 获取当前线程处理的请求的 msgid
        std::string msgid = RunTime::GetRunTime()->m_msgid;
        std::string method_name = RunTime::GetRunTime()->m_method_name;
        if (!msgid.empty())
        {
            ss << "[" << msgid << "]\t";
        }
        if (!method_name.empty())
        {
            ss << "[" << method_name << "]\t";
        }

        return ss.str();
    }
    void Logger::setLogLevel(std::string log_level_str)
    {
        m_log_level = StringToLoglevel(log_level_str);
    }

    void Logger::pushLog(const std::string &msg)
    {
        if(0==m_type){
            printf((msg + "\n").c_str());
            return;
        }
        std::unique_lock<std::mutex> ulk(m_mutex);
        m_buffer.push_back(msg);
    }
    void Logger::pushAppLog(const std::string &msg)
    {
        std::lock_guard<std::mutex> lkg(m_mutex);
        m_app_buffer.push_back(msg);
    }

    void Logger::log()
    {
        /*
        std::vector<std::string> temp;
        {
            std::unique_lock<std::mutex> ulk(m_mutex);
            temp.swap(m_buffer);
        }
        while (!temp.empty())
        {
            std::string msg = temp.front();
            temp.pop_back();
            printf(msg.c_str());
        }
        */
    }
    void Logger::InitGlobalLogger(int type /* = 0 */ ){
        m_type = type;
        if (m_type == 0)
        {
            // printf("m_type = %d\n", m_type);
            return;
        }
        m_asnyc_logger = std::make_shared<AsyncLogger>(
            Config::getInstance()->m_log_file_name + "_rpc",
            Config::getInstance()->m_log_file_path,
            Config::getInstance()->m_log_max_file_size);

        m_asnyc_app_logger = std::make_shared<AsyncLogger>(
            Config::getInstance()->m_log_file_name + "_app",
            Config::getInstance()->m_log_file_path,
            Config::getInstance()->m_log_max_file_size);

        // printf("m_type = %d\n", m_type);
        m_timer_event = std::make_shared<TimerEvent>(Config::getInstance()->m_log_sync_inteval, true, std::bind(&Logger::syncLoop, this));
        EventLoop::GetCurrentEventLoop()->addTimerEvent(m_timer_event);
    }
    void Logger::syncLoop()
    {
        // 同步 m_buffer 到 async_logger 的buffer队尾
        // printf("sync to async logger\n");
        std::vector<std::string> tmp_vec;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            tmp_vec.swap(m_buffer);
        }
        if (!tmp_vec.empty())
        {
            m_asnyc_logger->pushLogBuffer(tmp_vec);
        }
        tmp_vec.clear();
        // 同步 m_app_buffer 到 app_async_logger 的buffer队尾
        std::vector<std::string> tmp_vec2;
        {
            std::lock_guard<std::mutex> lock(m_app_mutex);
            tmp_vec2.swap(m_app_buffer);
        }
        if (!tmp_vec2.empty())
        {
            m_asnyc_app_logger->pushLogBuffer(tmp_vec2);
        }
    }

    //asynclogger:
    AsyncLogger::AsyncLogger(const std::string &file_name, const std::string &file_path, int max_size)
        : m_file_name(file_name), m_file_path(file_path), m_max_file_size(max_size)
    {
        sem_init(&m_sempahore, 0, 0);
        m_thread = std::thread(&AsyncLogger::Loop, this);
        assert(m_thread.joinable());
        // printf("sem wait before\n");
        sem_wait(&m_sempahore);
        // printf("sem wait after\n");
    }
    void *AsyncLogger::Loop(void *arg)
    {
        // 将 buffer 里面的全部数据打印到文件中，然后线程睡眠，直到有新的数据再重复这个过程
        AsyncLogger *logger = reinterpret_cast<AsyncLogger *>(arg);
        // printf("sem init before\n");
        //sem_init(&logger->m_sempahore, 0, 0);
        // printf("sem post before\n");
        sem_post(&logger->m_sempahore);
        // printf("sem post after\n");
        while (1)
        {
            std::unique_lock<std::mutex> ulk(logger->m_mutex);
            logger->m_condtion.wait(ulk, [&logger]()
                                    { return !logger->m_buffer.empty(); });
            std::vector<std::string> tmp;
            tmp.swap(logger->m_buffer.front());
            logger->m_buffer.pop();
            ulk.unlock();

            timeval now;
            gettimeofday(&now, NULL);
            struct tm now_time;
            localtime_r(&(now.tv_sec), &now_time);

            const char *format = "%Y%m%d";
            char date[32];
            strftime(date, sizeof(date), format, &now_time);
            if (std::string(date) != logger->m_date)
            {
                logger->m_no = 0;
                logger->m_reopen_flag = true;
                logger->m_date = std::string(date);
            }
            if (logger->m_file_hanlder == NULL)
            {
                logger->m_reopen_flag = true;
            }
            std::stringstream ss;
            ss << logger->m_file_path << logger->m_file_name << "_"
               << std::string(date) << "_log.";
            std::string log_file_name = ss.str() + std::to_string(logger->m_no);
            if (logger->m_reopen_flag)
            {
                if (logger->m_file_hanlder)
                {
                    fclose(logger->m_file_hanlder);
                }
                logger->m_file_hanlder = fopen(log_file_name.c_str(), "a");
                logger->m_reopen_flag = false;
            }
            if (ftell(logger->m_file_hanlder) > logger->m_max_file_size)
            {
                fclose(logger->m_file_hanlder);
                log_file_name = ss.str() + std::to_string(logger->m_no++);
                logger->m_file_hanlder = fopen(log_file_name.c_str(), "a");
                logger->m_reopen_flag = false;
            }
            for (auto &i : tmp)
            {
                if (!i.empty())
                {
                    fwrite(i.c_str(), 1, i.length(), logger->m_file_hanlder);
                }
            }
            fflush(logger->m_file_hanlder);
            if (logger->m_stop_flag)
            {
                return NULL;
            }
        }
        return NULL;
    }
    void AsyncLogger::stop()
    {
        m_stop_flag = true;
    }
    void AsyncLogger::flush()
    {
        if (m_file_hanlder)
        {
            fflush(m_file_hanlder);
        }
    }
    void AsyncLogger::pushLogBuffer(std::vector<std::string> &vec)
    {
        std::lock_guard<std::mutex> lkg(m_mutex);
        m_buffer.push(vec);
        m_condtion.notify_one();
    }
}