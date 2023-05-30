#ifndef ROCKET_NET_TCP_SERVER_H
#define ROCKET_NET_TCP_SERVER_H

#include "rocket/net/tcp/tcp_acceptor.h"
#include "rocket/net/eventloop.h"
#include "rocket/net/io_thread_group.h"
#include "rocket/common/log.h"
namespace rocket
{
    class TCPServer
    {
    private:
        TcpAcceptor::s_ptr m_acceptor;
        NetAddr::s_ptr m_local_addr;

        EventLoop *m_main_event_loop{NULL}; // main reactor

        IOThreadGroup *m_io_thread_group{NULL}; //

        FdEvent *listen_fd_event;
        int m_client_counts;

        void init();
        void onAccept(); // 链接操作
    public:
        TCPServer(NetAddr::s_ptr local_addr);
        ~TCPServer();

        void start();
    };

} // namespace rocket

#endif