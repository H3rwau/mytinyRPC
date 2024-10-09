#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>
#include "tinyRPC/common/singleton.h"

namespace tinyRPC
{
    class Config : public Singleton<Config>
    {
        friend class Singleton<Config>;

    public:
        static void setConfigPath(const char *xmlfile);
        std::string m_log_level;
        static std::string m_config_path;

        std::string m_log_file_name;
        std::string m_log_file_path;
        int m_log_max_file_size{0};
        int m_log_sync_inteval{0}; // 日志同步间隔，ms
        int m_port{0};
        int m_io_threads{0};

    private:
        Config();
        ~Config() = default;
    };

} // namespace tinyRPC

#endif // __CONFIG_H__
