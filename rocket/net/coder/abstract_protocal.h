#ifndef ROCKET_NET_AB_PROTOCAL_H
#define ROCKET_NET_AB_PROTOCAL_H

// #include <memory>
#include "rocket/net/tcp/tcp_buffer.h"
#include<string>

namespace rocket
{
    class AbstractProtocal : public std::enable_shared_from_this<AbstractProtocal>
    {
    private:
        /* data */
    public:
        // AbstractProtocal(/* args */);
        virtual ~AbstractProtocal(){};


        typedef std::shared_ptr<AbstractProtocal> s_ptr;
        
        std::string getMsgId(){
            return m_msg_id;
        };

        void setMsgId(const std::string& msg_id){
            m_msg_id = msg_id;
        };

        std::string m_msg_id; //请求号，唯一表示请求或相应

    // protected:
        
    };
    

    
} // namespace rocket


#endif