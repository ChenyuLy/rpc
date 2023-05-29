#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <memory>
#include "rocket/common/log.h"
#include "rocket/common/config.h"
#include "rocket/net/eventloop.h"
#include "rocket/net/timer.h"
#include "rocket/net/timer_event.h"
#include "rocket/net/io_thread.h"
#include "rocket/net/io_thread_group.h"

void test_io_thread(){
     int listenfd = socket(AF_INET,SOCK_STREAM,0);
    if(listenfd == -1){
        ERRORLOG("listenfd == -1");
        exit(0);
    }

    sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_port = htons(12345);
    addr.sin_family = AF_INET;
     inet_aton("127.0.0.1",&addr.sin_addr);
    int rt = bind(listenfd,reinterpret_cast<sockaddr*>(&addr),sizeof(addr));
    if(rt != 0){
        ERRORLOG("bind error");
        exit(1);
    }

    rt = listen(listenfd,100);
    if (rt != 0)
    {
        ERRORLOG("listen error");
        exit(1);
    }
    
    rocket::FdEvent event(listenfd);
    event.listen(rocket::FdEvent::IN_EVENT,[listenfd](){
        sockaddr_in peer_addr;
        socklen_t addr_len = sizeof(peer_addr);
        memset(&peer_addr,0,sizeof(peer_addr));
        int client = accept(listenfd,reinterpret_cast<sockaddr*>(&peer_addr),&addr_len);
        ;
        DEBUGLOG("succ get client fd[%d] [%s:%d]",client,inet_ntoa(peer_addr.sin_addr),ntohs(peer_addr.sin_port));
    });
    
    
    int i = 0;
    rocket::TimerEvent::s_ptr timer_event = std::make_shared<rocket::TimerEvent> (1000,true,[&i](){
        INFOLOG("trigger timer event, count = %d",i++);
    });


    

    // rocket::IOThread io_thread;
    // io_thread.getEventLoop()->addEpollEvent(&event);
    // io_thread.getEventLoop()->addTimerEvent(timer_event);
    // io_thread.start();

    // io_thread.join();

    rocket::IOThreadGroup io_thread_group(2);
    rocket::IOThread* io_thread = io_thread_group.getIOThread();
    io_thread->getEventLoop()->addEpollEvent(&event);
    io_thread->getEventLoop()->addTimerEvent(timer_event);

    rocket::IOThread* io_thread2 = io_thread_group.getIOThread();
    // io_thread2->getEventLoop()->addEpollEvent(&event);
    io_thread2->getEventLoop()->addTimerEvent(timer_event);

    io_thread_group.start();
    io_thread_group.join();
}

int main(int argc, char const *argv[])
{
    rocket::Config::SetGlobelConfig("conf/rocket.xml");

    rocket::Logger::InitGlobgalLogger();

    test_io_thread();

    // rocket::EventLoop* eventloop = new rocket::EventLoop();

    // int listenfd = socket(AF_INET,SOCK_STREAM,0);
    // if(listenfd == -1){
    //     ERRORLOG("listenfd == -1");
    //     exit(0);
    // }

    // sockaddr_in addr;
    // memset(&addr,0,sizeof(addr));
    // addr.sin_port = htons(12345);
    // addr.sin_family = AF_INET;
    //  inet_aton("127.0.0.1",&addr.sin_addr);
    // int rt = bind(listenfd,reinterpret_cast<sockaddr*>(&addr),sizeof(addr));
    // if(rt != 0){
    //     ERRORLOG("bind error");
    //     exit(1);
    // }

    // rt = listen(listenfd,100);
    // if (rt != 0)
    // {
    //     ERRORLOG("listen error");
    //     exit(1);
    // }
    
    // rocket::FdEvent event(listenfd);
    // event.listen(rocket::FdEvent::IN_EVENT,[listenfd](){
    //     sockaddr_in peer_addr;
    //     socklen_t addr_len = sizeof(peer_addr);
    //     memset(&peer_addr,0,sizeof(peer_addr));
    //     int client = accept(listenfd,reinterpret_cast<sockaddr*>(&peer_addr),&addr_len);
    //     ;
    //     DEBUGLOG("succ get client fd[%d] [%s:%d]",client,inet_ntoa(peer_addr.sin_addr),ntohs(peer_addr.sin_port));
    // });
    
    // eventloop->addEpollEvent(&event);
    // int i = 0;
    // rocket::TimerEvent::s_ptr timer_event = std::make_shared<rocket::TimerEvent> (1000,true,[&i](){
    //     INFOLOG("trigger timer event, count = %d",i++);
    // });

    // eventloop->addTimerEvent(timer_event);
    // eventloop -> loop();
    return 0;
}
