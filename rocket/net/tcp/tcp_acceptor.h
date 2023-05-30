#ifndef ROCKET_NET_TCP_TCP_ACCEPTOR_H
#define ROCKET_NET_TCP_TCP_ACCEPTOR_H
#include "rocket/net/tcp/net_addr.h"
#include "rocket/common/log.h"

namespace rocket
{
    class TcpAcceptor
    {
    private:
        NetAddr::s_ptr m_local_addr;//服务端监听的地址， addr -> ip:port
        int m_family{-1};
        int m_listenfd{-1};   //listenfd 监听套接字
    public:
        TcpAcceptor(NetAddr::s_ptr local_addr);
        ~TcpAcceptor();

        int accept();
    };
    

    



} // namespace rocket


#endif