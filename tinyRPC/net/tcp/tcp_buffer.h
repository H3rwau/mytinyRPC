#ifndef __TCP_BUFFER__
#define __TCP_BUFFER__
#include <vector>
#include <memory>

namespace tinyRPC
{
    class TcpBuffer
    {
    public:
        using s_ptr = std::shared_ptr<TcpBuffer>;
        TcpBuffer(int size);
        //~TcpBuffer();

        // 返回可读字节数
        int readAble();
        // 返回可写的字节数
        int writeAble();

        int readIndex();
        int writeIndex();

        void writeToBuffer(const char *buf, int size);
        void readFromBuffer(std::vector<char> &re, int size);

        // 扩容
        void resizeBuffer(int new_size);

        // 紧缩指针
        void adjustBuffer();
        // 移动读写指针
        void moveReadIndex(int size);
        void moveWriteIndex(int size);

    public:
        std::vector<char> m_buffer;

    private:
        int m_read_index{0};  // 读指针
        int m_write_index{0}; // 写指针
        int m_size{0};
    };
} // tinyRPC

#endif // __TCP_BUFFER__