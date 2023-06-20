#ifndef ROCKET_NET_RPC_DISPATCHER_H
#define ROCKET_NET_RPC_DISPATCHER_H

#include "../coder/abstract_protocal.h"
#include <memory>
#include <map>
#include <google/protobuf/service.h>
#include "rocket/net/coder/tinypb_protocol.h"

// #include "rocket/net/tcp/tcp_connection.h"
// #include "rocket/net/coder/abstract_protocal.h"

namespace rocket
{
    class TcpConnection;
    class RpcDispatcher
    {
    public:

    static RpcDispatcher* GetRpcDispatcher();

    

    typedef std::shared_ptr<google::protobuf::Service> service_s_ptr;
        void dispatch(AbstractProtocal::s_ptr request, AbstractProtocal::s_ptr response,TcpConnection* connection);

        void registerService(std::shared_ptr<google::protobuf::Service> service);

        void setTinyPBError(std::shared_ptr<TinyPBProtocal> msg , int32_t err_code,const std::string err_info);
    private:
        std::map<std::string, std::shared_ptr<google::protobuf::Service>> m_service_map;

        bool parseServiceFullName(const std::string &full_name ,std::string & service_name,std::string & method_name);
    };
}

#endif