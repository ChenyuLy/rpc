#ifndef ROCKET_NET_TCP_CLIENT_H
#define ROCKET_NET_TCP_CLIENT_H

#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/eventloop.h"
#include "rocket/common/log.h"
#include "rocket/net/tcp/tcp_connection.h"
#include "rocket/net/coder/abstract_protocal.h"
#include <unistd.h>
#include <memory>
namespace rocket
{
    class TcpClient
    {
    private:
        NetAddr::s_ptr m_peer_addr;
        NetAddr::s_ptr m_local_addr;
        EventLoop *m_event_loop{NULL};
        int m_fd{-1};
        FdEvent *m_fd_event{NULL};

        TcpConnection::s_ptr m_connection;

        int m_connect_errcode{0};
        std::string m_connect_error_info;

    public:
        TcpClient(NetAddr::s_ptr peer_addr);
        ~TcpClient();

        typedef std::shared_ptr<TcpClient> s_ptr;
        //如果connect执行完成会执行 func
        void connect(std::function<void()> done);//异步执行connect
        //异步的发送massage
        //如果发送message成功会调用done函数，函数的参数是message对象
        void writeMessage(AbstractProtocal::s_ptr message,std::function<void(AbstractProtocal::s_ptr)> done);

                //异步的发送massage
        //如果读取message成功会调用done函数，函数的参数是message对象
        void readMessage(const std::string& msg_id ,  std::function<void(AbstractProtocal::s_ptr)> done);

        void stop();

        int getConnectErrorCode();

        std::string  getConnectErrorInfo();

        NetAddr::s_ptr getPeerAddr();
        NetAddr::s_ptr getLocalAddr();

        void initLocalAddr();

    };
} // namespace rocket

#endif