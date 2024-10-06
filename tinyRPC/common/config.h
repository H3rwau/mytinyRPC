#if !defined(__CONFIG_H__)
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
        static std::string m_log_level;
        static std::string m_config_path;

    private:
        Config();
        ~Config() = default;
    };

} // namespace tinyRPC

#endif // __CONFIG_H__
