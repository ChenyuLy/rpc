// #include "rocket/net/eventloop.h"
// #include "rocket/common/log.h"
// #include "rocket/common/util.h"
// #include <sys/socket.h>
// #include <sys/epoll.h>

// namespace rocket
// {
    
// static thread_local EventLoop* t_current_eventloop = NULL;

// EventLoop::EventLoop(){
//     if(t_current_eventloop != NULL){
//         Error("failed to create event loop,  this thread has created event loop");
//         exit(0);
//     }
//     m_pid = getThreadId();
//     m_epoll_fd = epoll_create(10);
//     if(m_epoll_fd == -1 ){
//         ERRORLOG("failed to create event loop,  epoll_create rooro , error info[%S]", errno);
//         exit(0);
//     }

//     INFOLOG("succ create event loop in thread % d", m_pid);

//     t_current_eventloop = this;

// }
















// } // namespace rocket


















