#if !defined(__LOG_H__)
#define __LOG_H__
#include <string>
#include <sstream>
#include <memory>
#include <queue>
#include <stdio.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <semaphore.h>
#include <condition_variable>
#include <sys/time.h>
#include "tinyRPC/common/singleton.h"
#include "tinyRPC/net/timer_event.h"

namespace tinyRPC
{

    template <typename... Args>
    std::string formatString(const char *str, Args &&...args)
    {
        int size = snprintf(nullptr, 0, str, args...);
        std::string results;
        if (size > 0)
        {
            results.resize(size);
            snprintf(&results[0], size + 1, str, args...);
        }
        return results;
    }

#define prefix(file, line, msg)\
    (std::string("[").append(file).append(":").append(line).append("] ").append(msg))

#define DEBUGLOG(str, ...)\
    if (tinyRPC::Logger::m_log_level <= tinyRPC::Debug)\
    {\
        tinyRPC::Logger::getInstance()->pushLog(prefix(__FILE__, std::to_string(__LINE__),\
        tinyRPC::LogEvent(tinyRPC::Loglevel::Debug).toString() + tinyRPC::formatString(str, ##__VA_ARGS__) + "\n"));\
    }

#define INFOLOG(str, ...)\
    if (tinyRPC::Logger::m_log_level <= tinyRPC::Info)\
    {\
        tinyRPC::Logger::getInstance()->pushLog(prefix(__FILE__, std::to_string(__LINE__),\
        tinyRPC::LogEvent(tinyRPC::Loglevel::Info).toString() + tinyRPC::formatString(str, ##__VA_ARGS__) + "\n"));\
    }

#define ERRORLOG(str, ...)\
    if (tinyRPC::Logger::m_log_level <= tinyRPC::Error)\
    {\
        tinyRPC::Logger::getInstance()->pushLog(prefix(__FILE__, std::to_string(__LINE__),\
        tinyRPC::LogEvent(tinyRPC::Loglevel::Error).toString() + tinyRPC::formatString(str, ##__VA_ARGS__) + "\n")); \
    }

#define APPDEBUGLOG(str, ...)\
    if (tinyRPC::Logger::m_log_level <= tinyRPC::Debug)\
    {\
        tinyRPC::Logger::getInstance()->pushAppLog(prefix(__FILE__, std::to_string(__LINE__),\
        tinyRPC::LogEvent(tinyRPC::Loglevel::Debug).toString() + tinyRPC::formatString(str, ##__VA_ARGS__) + "\n"));\
    }

#define APPINFOLOG(str, ...)\
    if (tinyRPC::Logger::m_log_level <= tinyRPC::Info)\
    {\
        tinyRPC::Logger::getInstance()->pushAppLog(prefix(__FILE__, std::to_string(__LINE__),\
        tinyRPC::LogEvent(tinyRPC::Loglevel::Info).toString() + tinyRPC::formatString(str, ##__VA_ARGS__) + "\n"));\
    }

#define APPERRORLOG(str, ...)\
    if (tinyRPC::Logger::m_log_level <= tinyRPC::Error)\
    {\
        tinyRPC::Logger::getInstance()->pushAppLog(prefix(__FILE__, std::to_string(__LINE__),\
        tinyRPC::LogEvent(tinyRPC::Loglevel::Error).toString() + tinyRPC::formatString(str, ##__VA_ARGS__) + "\n")); \
    }

    enum Loglevel
    {
        Debug = 1,
        Info = 2,
        Error = 3,
        Unknown = 4,
    };
    std::string LoglevelToString(Loglevel level);
    Loglevel StringToLoglevel(std::string str);
    class LogEvent
    {
    private:
        std::string m_file_name; // 文件名
        int32_t m_file_line;     // 行号
        int32_t m_pid;           // 进程号
        int32_t m_thread_id;     // 线程号
        Loglevel m_level;        // 日志级别

    public:
        LogEvent(Loglevel level) : m_level(level) {}
        std::string toString();
        std::string getFileName() const
        {
            return m_file_name;
        }
        Loglevel getFileLevel() const
        {
            return m_level;
        }
    };

    class AsyncLogger
    {
    public:
        using s_ptr =  std::shared_ptr<AsyncLogger>;
        AsyncLogger(const std::string &file_name, const std::string &file_path, int max_size);
        void stop();
        // 刷新到磁盘
        void flush();
        void pushLogBuffer(std::vector<std::string> &vec);

    public:
        static void *Loop(void *);

    private:
        // m_file_path/m_file_name_yyyymmdd.0
        std::queue<std::vector<std::string>> m_buffer;//将日志内容插入到队列里
        std::string m_file_name; // 日志输出文件名字
        std::string m_file_path; // 日志输出路径
        int m_max_file_size{0};  // 日志单个文件最大大小, 单位为字节
        sem_t m_sempahore;
        std::thread m_thread;
        std::condition_variable m_condtion; // 条件变量
        std::mutex m_mutex;
        std::string m_date;         // 当前打印日志的文件日期
        FILE *m_file_hanlder{NULL}; // 当前打开的日志文件句柄
        bool m_reopen_flag{false};
        int m_no{0}; // 日志文件序号
        bool m_stop_flag{false};
    };

    class Logger
        : public Singleton<Logger>
    {
        friend class Singleton<Logger>;

    public:
        static void setLogLevel(std::string log_level_str);
        void InitGlobalLogger(int type = 0);
        // typedef std::shared_ptr<Logger> s_ptr;
        void pushLog(const std::string &msg);
        void pushAppLog(const std::string &msg);
        // void init();
        void log();
        void syncLoop();

    public:
        static Loglevel m_log_level;

    private:
        Logger() {}
        // Loglevel m_set_level;
        std::vector<std::string> m_buffer;
        std::vector<std::string> m_app_buffer;
        std::mutex m_mutex;
        std::mutex m_app_mutex;

        std::string m_file_name; // 日志输出文件名字
        std::string m_file_path; // 日志输出路径
        int m_max_file_size{0};  // 日志单个文件最大大小
        AsyncLogger::s_ptr m_asnyc_logger;
        AsyncLogger::s_ptr m_asnyc_app_logger;
        TimerEvent::s_ptr m_timer_event;
        int m_type{0};
    };
}
#endif // __LOG_H__
