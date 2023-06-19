#include "tcp_client.h"
#include <string.h>
namespace rocket
{
    TcpClient::TcpClient(NetAddr::s_ptr peer_addr) : m_peer_addr(peer_addr)
    {
        m_event_loop = EventLoop::GetCurrentEventLoop();
        m_fd = socket(peer_addr->intGetFamily(), SOCK_STREAM, 0);

        if (m_fd < 0)
        {
            ERRORLOG("TcpClient::TcpClient() errpr , failed to create fd ");
            return;
        }

        m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);
        m_connection = std::make_shared<TcpConnection>(m_event_loop, m_fd, 128, peer_addr,nullptr,TcpConnection::TcpConnectionType::TcpConnectionByClient);
        m_connection->setConnectionType(TcpConnection::TcpConnectionType::TcpConnectionByClient);
    }

    TcpClient::~TcpClient()
    {
        DEBUGLOG("~TcpClient");
        if (m_fd > 0)
        {
            close(m_fd);
        }
    }

    void TcpClient::connect(std::function<void()> done)
    {
        int rt = ::connect(m_fd, m_peer_addr->getSocketAddr(), m_peer_addr->getSocketLen());
        if (rt == 0)
        {
            DEBUGLOG("connect [%s] success 0", m_peer_addr->toString().c_str());
            if (done)
            {
                done();
            }
            m_fd_event->cancle(FdEvent::OUT_EVENT);
            m_event_loop->addEpollEvent(m_fd_event);
        }
        else if (rt == -1)
        {
            if (errno == EINPROGRESS)
            {
                m_fd_event->listen(FdEvent::OUT_EVENT, [this, done](){
                    int error = 0;
                    socklen_t error_len = sizeof(error);
                    getsockopt(m_fd,SOL_SOCKET,SO_ERROR,&error,&error_len);
                    bool is_connect_succ = false;
                    if(error == 0){
                        DEBUGLOG("connect [%s] success 1",m_peer_addr->toString().c_str());
                        is_connect_succ = true;
                        m_connection->setState(TcpConnection::TcpState::Connected);

                    } else{
                        ERRORLOG("connect error, errno = %d, error=%s",errno,strerror(errno));
                    }
                    //链接完成后去掉可写触发
                    m_fd_event->cancle(FdEvent::OUT_EVENT);
                    m_event_loop->addEpollEvent(m_fd_event);

                    //如果链接成功才会执行回调函数
                    if(done&& is_connect_succ){
                            
                            done();
                    } 
                });

                m_event_loop->addEpollEvent(m_fd_event);
                if (!m_event_loop->isLooping())
                    m_event_loop->loop();
            }
            else
            {
                ERRORLOG("connect error, errno = %d, error=%s", errno, strerror(errno));
            }
        }
    }

    void TcpClient::writeMessage(AbstractProtocal::s_ptr message, std::function<void(AbstractProtocal::s_ptr)> done)
    {
        // 把message 对象写入到Connection 的 buffer中 done也写入
        // 启动connection可写事件
        m_connection->pushSendMessage(message, done);
        m_connection->listenWrite();
    }

    void TcpClient::readMessage(const std::string& req_id , std::function<void(AbstractProtocal::s_ptr)> done)
    {
        //1监听可读事件
        //2从buffer里decode得到message 对象 判断是否热情——id相等，相等则成功，执行回调
        m_connection->pushReadMessage(req_id,done);
        m_connection->listenRead();
    }

} // namespace rocket
