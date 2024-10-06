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

    std::string Config::m_log_level;
    std::string Config::m_config_path;
    void Config::setConfigPath(const char *xmlfile)
    {
        m_config_path = std::string(xmlfile);
    }

    Config::Config()
    {
        tinyxml2::XMLDocument *xml_doc = new tinyxml2::XMLDocument();
        tinyxml2::XMLError error = xml_doc->LoadFile(m_config_path.c_str());
        if (error != tinyxml2::XMLError::XML_SUCCESS)
        {
            printf("start server error, failed to read config file %s\n", m_config_path.c_str());
            exit(0);
        }
        READ_XML_NODE(root, xml_doc);
        READ_XML_NODE(log, root);
        READ_STR_FROM_XML_NODE(log_level, log); // 得到了log_level_str
        m_log_level = log_level_str;
    }
}