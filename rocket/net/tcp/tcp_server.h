#ifndef ROCKET_NET_TCP_SERVER_H
#define ROCKET_NET_TCP_SERVER_H

#include <set>
#include "rocket/net/tcp/tcp_acceptor.h"
#include "rocket/net/eventloop.h"
#include "rocket/net/io_thread_group.h"
#include "rocket/common/log.h"
#include "rocket/net/tcp/tcp_connection.h"
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
        int m_client_counts{0};

        std::set<TcpConnection::s_ptr> m_client;

        void init();
        void onAccept(); // 链接操作
    public:
        TCPServer(NetAddr::s_ptr local_addr);
        ~TCPServer();

        void start();
    };

} // namespace rocket

#endif