#ifndef __ABSTRACT_PROTOCOL__
#define __ABSTRACT_PROTOCOL__

#include <memory>

namespace tinyRPC
{
    class AbstractProtocol 
    {
    public:
        using s_ptr = std::shared_ptr<AbstractProtocol>;

    private:
        
    };
} // tinyRPC

#endif // __ABSTRACT_PROTOCOL__