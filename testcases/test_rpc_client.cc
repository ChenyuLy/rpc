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
#include "rocket/net/rpc/rpc_controller.h"
#include "rocket/net/rpc/rpc_channel.h"
#include "rocket/net/rpc/rpc_closure.h"
#include "order.pb.h"
#include <memory>

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

        message->m_msg_id = "999888";

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
            DEBUGLOG("msg_id[%s], get response %s",message->m_msg_id.c_str(),message->m_pb_data.c_str());
            makeOrderResponse response;

            if(!response.ParseFromString(message->m_pb_data)){
                ERRORLOG("deserilize error");
                return;
            }
            DEBUGLOG("get response success, response[%s]",response.ShortDebugString().c_str());


        });


        });
}

void test_rpc_channel(){
    // std::string ip = "127.0.0.1";
    // rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>(ip, 12345);
    // std::shared_ptr<rocket::RpcChannel> channel = std::make_shared<rocket::RpcChannel>(addr);

    NEWRPCCHANNEL("127.0.0.1",channel);
    NEWMESSAGE(makeOrderRequest,request);
    NEWMESSAGE(makeOrderResponse,response);
    // std::shared_ptr<makeOrderRequest> request = std::make_shared<makeOrderRequest>();
    request->set_price(100);
    request->set_goods("apple");

    // std::shared_ptr<makeOrderResponse> response = std::make_shared<makeOrderResponse>();

    // std::shared_ptr<rocket::RpcController> controller = std::make_shared<rocket::RpcController>();
    NEWRPCCONTROLLER(controller);
    controller->SetMsgId("99998888");
    
    std::shared_ptr<rocket::RpcClosure> closure = std::make_shared<rocket::RpcClosure>([request,response,channel,controller]() mutable{
        if(controller->GetErrorCode()==0){
            INFOLOG("call rpc sucess,request[%s],response[%s]",request->ShortDebugString().c_str(),response->ShortDebugString().c_str());
            //业务逻辑
            if(response->order_id() == "xxx"){

            }  
        } else{
            INFOLOG("call rpc faile,request[%s],error code[%d] , error indo [%s]",
                request->ShortDebugString().c_str(),
                controller->GetErrorCode(),
                controller->GetErrorInfo().c_str());
        }
        
        INFOLOG("exit eventloop");
        channel->getTcpClient()->stop();
        channel.reset();
    });

    // channel->Init(controller,response,request,closure);
    // Order_Stub stub(channel.get());
    // // controller->setTimeout(5000);
    // stub.makeOrder(controller.get(),request.get(),response.get(),closure.get());

    CALLRCP("127.0.0.1:12345",makeOrder,controller,response,request,closure);
}

int main()
{
    rocket::Config::SetGlobelConfig("conf/rocket.xml");
    rocket::Logger::InitGlobgalLogger();

    // std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    // rocket::RpcDispatcher::GetRpcDispatcher()->registerService(service);


    // test_tcp_client();
    test_rpc_channel();
    return 0;
}