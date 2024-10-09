#ifndef __RUN_TIME__
#define __RUN_TIME__

#include <string>

namespace tinyRPC
{
    class RunTime
    {
    public:
        static RunTime *GetRunTime();

    public:
        std::string m_msgid;
        std::string m_method_name;
    };
} // tinyRPC

#endif // __RUN_TIME__