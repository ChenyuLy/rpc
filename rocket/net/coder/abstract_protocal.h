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
        
        std::string getReqID(){
            return m_req_id;
        };

        void setReqId(const std::string& req_id){
            m_req_id = req_id;
        };

        std::string m_req_id; //请求号，唯一表示请求或相应

    // protected:
        
    };
    

    
} // namespace rocket


#endif