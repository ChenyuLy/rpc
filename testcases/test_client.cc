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


void test_connect()
{
    // 调用connect 链接server
    // write一个字符串
    // 等待read返回结果

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        ERRORLOG("invalid fd %d", fd);
        exit(0);
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_aton("127.0.0.1", &server_addr.sin_addr);

    int rt = connect(fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr));
    std::string msg = "hello socket!";

    rt = write(fd, msg.c_str(), msg.length());

    DEBUGLOG("succ write %d bytes, [%s]", rt, msg.c_str());

    char buf[100];

    rt = read(fd, buf, 100);
    DEBUGLOG("success read %d bytes,[%s]", rt, std::string(buf).c_str());
}


void test_tcp_client()
{

    std::string ip = "127.0.0.1";
    rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>(ip, 12345);
    rocket::TcpClient client(addr);
    client.connect([addr, &client]()
                   {
        DEBUGLOG("connect to  [%s] success",addr->toString().c_str());
        // std::shared_ptr< rocket::StringProtocal>  message = std::make_shared<rocket::StringProtocal>();
        std::shared_ptr< rocket::TinyPBProtocal>  message = std::make_shared<rocket::TinyPBProtocal>();
        message->m_pb_data = "test pb data";

        message->m_msg_id = "123456789";
        client.writeMessage(message,[](rocket::AbstractProtocal::s_ptr msg_ptr ){
            DEBUGLOG("send message success");
        
        });

        client.readMessage("123456789",[](rocket::AbstractProtocal::s_ptr msg_ptr ){
            std::shared_ptr<rocket::TinyPBProtocal> message = std::dynamic_pointer_cast<rocket::TinyPBProtocal>(msg_ptr);
            DEBUGLOG("msg_id[%s], get response %s",message->m_msg_id.c_str(),message->m_pb_data.c_str());
        
        });

        // client.writeMessage(message,[](rocket::AbstractProtocal::s_ptr msg_ptr ){
        //     DEBUGLOG("send message 2222222222222 success");
        
        // }); 
        });
}



void test_rpc_channel(){

}

int main()
{
    rocket::Config::SetGlobelConfig("conf/rocket.xml");
    rocket::Logger::InitGlobgalLogger();
    // test_connect();
    test_tcp_client();
    return 0;
}