#include "rocket/common/log.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_server.h"
#include <memory>


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

    test_tcp_server();
    
}