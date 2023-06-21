#ifndef ROCKET_NET_STRING_CODER_H
#define ROCKET_NET_STRING_CODER_H

#include "rocket/net/coder/abstract_coder.h"
#include "rocket/net/coder/abstract_protocal.h"
#include <string>
namespace rocket
{

    class StringProtocal : public AbstractProtocal
    {
        public:
        std::string info;

    };

    class StringCoder : public AbstractCoder
    {
    private:
    public:
        // StringCoder(){};
        // ~StringCoder(){};

        // 将messge对象转化为字节流，写入到buffer
        virtual void encode(std::vector<AbstractProtocal::s_ptr> &messages, TcpBuffer::s_ptr out_buffer)
        {
            for(size_t i = 0; i<messages.size();++i){
                std::shared_ptr<StringProtocal> msg = std::dynamic_pointer_cast<StringProtocal>(messages[i]);
                out_buffer->writeToBuffer(msg->info.c_str(),msg->info.length());
            }
        }
        // 将buffer对象转化为字message
        virtual void decode(std::vector<AbstractProtocal::s_ptr> &out_messages, TcpBuffer::s_ptr buffer)
        {
            std::vector<char> re;
            buffer->readFromBuffer(re,buffer->readAble());
            std::string info;
            for(size_t i=0 ;i<re.size();++i){
                info += re[i];
            }

            std::shared_ptr<StringProtocal> msg= std::make_shared< StringProtocal>();
            msg->info = info;
            msg->setMsgId("123456");
            out_messages.push_back(msg);


        }
    };

} // namespace rocket

#endif