#ifndef ROCKET_NET_TCP_CLIENT_H
#define ROCKET_NET_TCP_CLIENT_H

#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/eventloop.h"
#include "rocket/common/log.h"
#include "rocket/net/tcp/tcp_connection.h"
#include "rocket/net/abstract_protocal.h"
#include <unistd.h>
namespace rocket
{
    class TcpClient
    {
    private:
        NetAddr::s_ptr m_peer_addr;
        EventLoop *m_event_loop{NULL};
        int m_fd{-1};
        FdEvent *m_fd_event{NULL};

        TcpConnection::s_ptr m_connection;

    public:
        TcpClient(NetAddr::s_ptr peer_addr);
        ~TcpClient();

        //如果connect执行成功会执行 func
        void connect(std::function<void()> done);//异步执行connect
        //异步的发送massage
        //如果发送message成功会调用done函数，函数的参数是message对象
        void writeMessage(AbstractProtocal::s_ptr message,std::function<void(AbstractProtocal::s_ptr)> done);

                //异步的发送massage
        //如果读取message成功会调用done函数，函数的参数是message对象
        void readMessage(AbstractProtocal::s_ptr message,std::function<void(AbstractProtocal::s_ptr)> done);
    };
} // namespace rocket

#endif