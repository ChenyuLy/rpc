#include "rocket/common/log.h"
#include "rocket/common/config.h"
#include <pthread.h>
#include <assert.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include "rocket/net/tcp/tcp_acceptor.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>
#include <unistd.h>
#include "rocket/net/tcp/tcp_client.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/coder/string_coder.h"
#include "rocket/net/coder/abstract_protocal.h"
#include "rocket/net/coder/tinypb_coder.h"
#include "rocket/net/coder/tinypb_protocol.h"
#include <google/protobuf/service.h>
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_server.h"
#include "rocket/net/rpc/rpc_dispatcher.h"
#include "order.pb.h"

void test_tcp_client()
{

    std::string ip = "127.0.0.1";
    rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>(ip, 12345);
    rocket::TcpClient client(addr);
    client.connect([addr, &client]()
                   {
        DEBUGLOG("connect to  [%s] success",addr->toString().c_str());

        std::shared_ptr< rocket::TinyPBProtocal>  message = std::make_shared<rocket::TinyPBProtocal>();
        message->m_pb_data = "test pb data";

        message->m_req_id = "999888";

        makeOrderRequest request;
        request.set_price(100);
        request.set_goods("apple");

        if(!request.SerializeToString(&(message->m_pb_data))){
            ERRORLOG("serilize error");
            return;
        }

        message->m_method_name = "Order.makeOrder";


        client.writeMessage(message,[request](rocket::AbstractProtocal::s_ptr msg_ptr ){
            DEBUGLOG("send message success,resquest[%s]",request.ShortDebugString().c_str());
        
        });

        client.readMessage("999888",[](rocket::AbstractProtocal::s_ptr msg_ptr ){
            std::shared_ptr<rocket::TinyPBProtocal> message = std::dynamic_pointer_cast<rocket::TinyPBProtocal>(msg_ptr);
            DEBUGLOG("req_id[%s], get response %s",message->m_req_id.c_str(),message->m_pb_data.c_str());
            makeOrderResponse response;

            if(!response.ParseFromString(message->m_pb_data)){
                ERRORLOG("deserilize error");
                return;
            }
            DEBUGLOG("get response success, response[%s]",response.ShortDebugString().c_str());


        });


        });
}

int main()
{
    rocket::Config::SetGlobelConfig("conf/rocket.xml");
    rocket::Logger::InitGlobgalLogger();

    // std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    // rocket::RpcDispatcher::GetRpcDispatcher()->registerService(service);


    test_tcp_client();
    return 0;
}