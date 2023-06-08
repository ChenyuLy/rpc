#ifndef ROCKET_NET_ABSTRACT_CODER_H
#define ROCKET_NET_ABSTRACT_CODER_H

#include <vector>
#include "rocket/net/tcp/tcp_buffer.h"
#include "rocket/net/abstract_protocal.h"
namespace rocket
{
    class AbstractCoder
    {
    public:
    //将messge对象转化为字节流，写入到buffer
        virtual void encode(std::vector<AbstractProtocal::s_ptr >& message,TcpBuffer::s_ptr out_buffer) = 0;
    //将buffer对象转化为字message
        virtual void decode(std::vector<AbstractProtocal::s_ptr>& out_message,TcpBuffer::s_ptr buffer) = 0;


    public:
        // AbstractCoder(/* args */);
        virtual ~AbstractCoder(){};
    };


    

} // namespace rocket

#endif // ROKET_NET_ABSTRACT_CODER_H
