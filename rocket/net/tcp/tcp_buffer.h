#ifndef ROCKET_NET_EVENTLOOP_H
#define ROCKET_NET_EVENTLOOP_H

#include <vector>
namespace rocket
{
    class TcpBuffer
    {
    private:
        int m_read_index{0};
        int m_write_index{0};
        int m_size{0};

        std::vector<char> m_buffer;

    public:
        TcpBuffer(int size);
        ~TcpBuffer();

        int readAble();

        int writeAble();

        int readIndex();

        int writeIndex();

        void writeToBuffer(const char* buf,int size);

        void readFromBuffer(std::vector<char>& re,int size);

        void resizeBuffer(int new_size);

        void adjustBuffer();

        void moveReadIndex(int size);

        void moveWriteIndex(int size);
    };
    
    
} // namespace rocket


#endif