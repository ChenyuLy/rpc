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
#include "order.pb.h"
#include <google/protobuf/service.h>
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_server.h"
#include "rocket/net/rpc/rpc_dispatcher.h"

class OrderImpl: public Order{
    public:
      virtual void makeOrder(google::protobuf::RpcController* controller,
                       const ::makeOrderRequest* request,
                       ::makeOrderResponse* response,
                       ::google::protobuf::Closure* done){

            APPDEBUGLOG("start sleep 5");
            sleep(5);
            APPDEBUGLOG("end sleep 5");
            if(request -> price() <10 ){
                response->set_ret_code(-1);
                response->set_res_info("short balence");
                return;
            }
            response->set_order_id("20230514");
            APPDEBUGLOG("call makeOrder success");

        }
};



void test_tcp_server(){

    std::string ip = "127.0.0.1";
    rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>(ip,12345);

    DEBUGLOG("create addr %s",addr->toString().c_str());
    rocket::TCPServer tcp_server(addr);

    tcp_server.start();
}

int main()
{
    rocket::Config::SetGlobelConfig("conf/rocket.xml");
    rocket::Logger::InitGlobgalLogger();

    std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    rocket::RpcDispatcher::GetRpcDispatcher()->registerService(service);


    test_tcp_server();
    return 0;
}