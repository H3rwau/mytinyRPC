#ifndef __SINGLETON__
#define __SINGLETON__

namespace tinyRPC
{
    template <typename T>
    class Singleton
    {
    public:
        static T *getInstance()
        {
            static T instance; // c++11的static变量具有线程安全特性
            return &instance;
        }

        Singleton(const Singleton &) = delete;
        Singleton &operator=(const Singleton &) = delete;
        Singleton(Singleton &&) = delete;
        Singleton &operator=(Singleton &&) = delete;

    protected:
        Singleton(/* args */)
        {
        }

        virtual ~Singleton()
        {
        }
    };
}
#endif // __SINGLETON__