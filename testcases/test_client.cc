#include "rocket/common/log.h"
#include "rocket/common/config.h"
#include <pthread.h>
#include <assert.h>
#include <sys/socket.h>
#include <fcntl.h>
#include<string.h>
#include "rocket/net/tcp/tcp_acceptor.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>
#include <unistd.h>



void test_connect(){
    //调用connect 链接server
    //write一个字符串
    //等待read返回结果


    int fd  = socket(AF_INET,SOCK_STREAM,0);
    if (fd < 0){
        ERRORLOG("invalid fd %d",fd);
        exit(0);
    }

    sockaddr_in server_addr ;
    memset(&server_addr,0,sizeof(server_addr)); 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_aton("127.0.0.1",&server_addr.sin_addr);

    int rt = connect(fd,reinterpret_cast<sockaddr*>(&server_addr),sizeof(server_addr));
    std::string msg = "hello socket!";

    rt = write(fd,msg.c_str(),msg.length());

    DEBUGLOG("succ write %d bytes, [%s]",rt,msg.c_str());

    char buf[100];

    rt =read(fd,buf,100);
    DEBUGLOG("success read %d bytes,[%s]",rt,std::string(buf).c_str());
} 



int main()
{
    rocket::Config::SetGlobelConfig("conf/rocket.xml");
    rocket::Logger::InitGlobgalLogger();
    test_connect();
    return 0;
}