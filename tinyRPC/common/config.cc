#include "tinyRPC/common/config.h"
#include "tinyRPC/common/tinyxml2.h"

#define READ_XML_NODE(name, parent)                                      \
    tinyxml2::XMLElement *name = parent->FirstChildElement(#name);       \
    if (!(name))                                                         \
    {                                                                    \
        printf("start server error, failed to read node [%s]\n", #name); \
        exit(0);                                                         \
    }
#define READ_STR_FROM_XML_NODE(name, parent)                             \
    tinyxml2::XMLElement *name = parent->FirstChildElement(#name);       \
    if (!name)                                                           \
    {                                                                    \
        printf("start server error, failed to read node [%s]\n", #name); \
        exit(0);                                                         \
    }                                                                    \
    std::string name##_str = std::string(name->GetText());

namespace tinyRPC
{

    std::string Config::m_config_path;

    void Config::setConfigPath(const char *xmlfile)
    {
        if(xmlfile)
            m_config_path = std::string(xmlfile);
    }

    Config::Config()
    {

        if (m_config_path==""){
            m_log_level = "DEBUG";
            return;
        }
        tinyxml2::XMLDocument *xml_doc = new tinyxml2::XMLDocument();
        tinyxml2::XMLError error = xml_doc->LoadFile(m_config_path.c_str());
        if (error != tinyxml2::XMLError::XML_SUCCESS)
        {
            printf("start server error, failed to read config file %s\n", m_config_path.c_str());
            exit(0);
        }
        READ_XML_NODE(root, xml_doc);
        READ_XML_NODE(log, root);
        READ_XML_NODE(server, root);
        READ_STR_FROM_XML_NODE(log_level, log); // 得到了log_level_str
        READ_STR_FROM_XML_NODE(log_file_name, log); // 得到了log_level_str
        READ_STR_FROM_XML_NODE(log_file_path, log); // 得到了log_level_str
        READ_STR_FROM_XML_NODE(log_max_file_size, log); // 得到了log_level_str
        READ_STR_FROM_XML_NODE(log_sync_interval, log); // 得到了log_level_str
        m_log_level = log_level_str;
        m_log_file_name = log_file_name_str;
        m_log_file_path = log_file_path_str;
        m_log_max_file_size = std::atoi(log_max_file_size_str.c_str());
        m_log_sync_inteval = std::atoi(log_sync_interval_str.c_str());

        printf("LOG -- CONFIG LEVEL[%s], FILE_NAME[%s],FILE_PATH[%s] MAX_FILE_SIZE[%d B], SYNC_INTEVAL[%d ms]\n",
               m_log_level.c_str(), m_log_file_name.c_str(), m_log_file_path.c_str(), m_log_max_file_size, m_log_sync_inteval);

        READ_STR_FROM_XML_NODE(port, server);
        READ_STR_FROM_XML_NODE(io_threads, server);

        m_port = std::atoi(port_str.c_str());
        m_io_threads = std::atoi(io_threads_str.c_str());
        printf("Server -- PORT[%d], IO Threads[%d]\n", m_port, m_io_threads);
    }
}