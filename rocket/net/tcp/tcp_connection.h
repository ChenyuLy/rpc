#ifndef ROCKET_NET_TCP_TCP_CONNETCTION_H
#define ROCKET_NET_TCP_TCP_CONNETCTION_H

#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_buffer.h"
#include "rocket/net/io_thread.h"
#include "rocket/net/fd_event_group.h"
#include "rocket/common/log.h"
#include "rocket/net/coder/abstract_protocal.h"
// #include "rocket/net/coder/string_coder.h"
#include "rocket/net/coder/string_coder.h"
#include "rocket/net/coder/abstract_coder.h"
#include "rocket/net/rpc/rpc_dispatcher.h"
#include <map>
#include <queue>
#include <unistd.h>
#include <memory>

namespace rocket
{
    class TcpConnection
    {
    public:
        enum TcpState
        {
            NotConnected = 1,
            Connected = 2,
            HalfClosing = 3,
            Closed = 4
        };

        enum TcpConnectionType{
            TcpConnectionByServer = 1,  //作为服务端使用，带便对端客户端的链接
            TcpConnectionByClient = 2,  //作为客户端使用，带便对端服务端的链接
        };
    private:
        EventLoop* m_event_loop{NULL}; // 代表持有该链接的io线程
        int m_fd{0};
        NetAddr::s_ptr m_local_addr;
        NetAddr::s_ptr m_peer_addr;

        TcpBuffer::s_ptr m_in_buffer;
        TcpBuffer::s_ptr m_out_buffer;

        FdEvent *m_fd_event{NULL}; //

        TcpState m_stat;
        

    public:
        // TcpConnection(IOThread *io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr);
        TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr,NetAddr::s_ptr m_local_addr,TcpConnectionType type = TcpConnectionByServer);
        ~TcpConnection();

        void setConnectionType(TcpConnectionType type);

        void onRead();

        void excute();

        void onWrite();

        void setState(const TcpConnection::TcpState state);

        TcpConnection::TcpState getState();

        typedef std::shared_ptr<TcpConnection> s_ptr;

        void clear();

        void shutdown(); // 主动关闭

        void listenWrite();//启动监听可写事件

        void listenRead();//启动监听可du事件

        void pushSendMessage(AbstractProtocal::s_ptr message,std::function<void(AbstractProtocal::s_ptr)> done);

        void pushReadMessage(const std::string& req_id,std::function<void(AbstractProtocal::s_ptr)> done);

        NetAddr::s_ptr getLocalAddr();
        NetAddr::s_ptr getPeerAddr();

        TcpConnectionType m_connection_type {TcpConnectionByServer};

        
        std::vector<std::pair<AbstractProtocal::s_ptr,std::function<void(AbstractProtocal::s_ptr)>>> m_write_dones;
        // key = AbstratProteocol.m_req_id
        std::map<std::string,std::function<void(AbstractProtocal::s_ptr)>> m_read_dones;

        AbstractCoder* m_coder {NULL};


        // std::shared_ptr<RpcDispatcher> m_dispatcher;

    };

}
#endif