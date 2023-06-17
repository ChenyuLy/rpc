#ifndef ROCKET_NET_CODER_TINYPB_CODER_H
#define ROCKET_NET_CODER_TINYPB_CODER_H

#include "rocket/net/coder/abstract_coder.h"
#include "rocket/net/coder/tinypb_protocol.h"

namespace rocket
{
    class TinyPBCoder : public AbstractCoder
    {
        public:
        // 将messge对象转化为字节流，写入到buffer
        void encode(std::vector<AbstractProtocal::s_ptr> &messages, TcpBuffer::s_ptr out_buffer);
        // 将buffer对象转化为字message
        void decode(std::vector<AbstractProtocal::s_ptr> &out_message, TcpBuffer::s_ptr buffer);

        ~TinyPBCoder(){};
        TinyPBCoder(){};
        private:
        const char * encodeTinyPB(std::shared_ptr<TinyPBProtocal> message,int & len);
        
    };
} // namespace rocket

#endif