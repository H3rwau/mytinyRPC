#if !defined(__LOG_H__)
#define __LOG_H__
#include <string>
#include <sstream>
#include <memory>
#include <queue>
#include <stdio.h>
#include <iostream>
#include <mutex>
#include <sys/time.h>
#include "tinyRPC/common/singleton.h"

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

#define prefix(file, line, msg) \
    (std::string("[").append(file).append(":").append(line).append("] ").append(msg))

#define DEBUGLOG(str, ...)                                                                                                                                          \
    if (tinyRPC::Logger::m_log_level <= tinyRPC::Debug)                                                                                                             \
    {                                                                                                                                                               \
        tinyRPC::Logger::getInstance()->pushLog(prefix(__FILE__, std::to_string(__LINE__),                                                                          \
                                                       tinyRPC::LogEvent(tinyRPC::Loglevel::Debug).toString() + tinyRPC::formatString(str, ##__VA_ARGS__) + "\n")); \
        tinyRPC::Logger::getInstance()->log();                                                                                                                      \
    }

#define INFOLOG(str, ...)                                                                                                                                          \
    if (tinyRPC::Logger::m_log_level <= tinyRPC::Info)                                                                                                             \
    {                                                                                                                                                              \
        tinyRPC::Logger::getInstance()->pushLog(prefix(__FILE__, std::to_string(__LINE__),                                                                         \
                                                       tinyRPC::LogEvent(tinyRPC::Loglevel::Info).toString() + tinyRPC::formatString(str, ##__VA_ARGS__) + "\n")); \
        tinyRPC::Logger::getInstance()->log();                                                                                                                     \
    }

#define ERRORLOG(str, ...)                                                                                                                                          \
    if (tinyRPC::Logger::m_log_level <= tinyRPC::Error)                                                                                                             \
    {                                                                                                                                                               \
        tinyRPC::Logger::getInstance()->pushLog(prefix(__FILE__, std::to_string(__LINE__),                                                                          \
                                                       tinyRPC::LogEvent(tinyRPC::Loglevel::Error).toString() + tinyRPC::formatString(str, ##__VA_ARGS__) + "\n")); \
        tinyRPC::Logger::getInstance()->log();                                                                                                                      \
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

    class Logger
        : public Singleton<Logger>
    {
        friend class Singleton<Logger>;

    public:
        static void setLogLevel(std::string log_level_str);
        // typedef std::shared_ptr<Logger> s_ptr;
        void pushLog(const std::string &msg);
        void log();
        static Loglevel m_log_level;

    private:
        Logger() {}
        // Loglevel m_set_level;
        std::queue<std::string> m_buffer;
        std::mutex mutex;
    };
}
#endif // __LOG_H__
