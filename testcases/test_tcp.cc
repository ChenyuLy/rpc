#include "rocket/common/log.h"
#include "rocket/net/tcp/net_addr.h"

int main()
{
    rocket::Config::SetGlobelConfig("conf/rocket.xml");
    rocket::Logger::InitGlobgalLogger();

    std::string ip = "127.0.0.1";
    rocket::IPNetAddr addr(ip,12345);
    DEBUGLOG("create addr %s",addr.toString().c_str());
}