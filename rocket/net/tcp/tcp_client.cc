#include "tcp_client.h"
#include <string.h>
namespace rocket
{
    TcpClient::TcpClient(NetAddr::s_ptr peer_addr):m_peer_addr(peer_addr)
    {
        m_event_loop = EventLoop::GetCurrentEventLoop();
        m_fd = socket(peer_addr -> intGetFamily(),SOCK_STREAM,0);

        if(m_fd < 0){
            ERRORLOG("TcpClient::TcpClient() errpr , failed to create fd ");
            return ;
        }

        m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);
        m_connection = std::make_shared<TcpConnection> (m_event_loop,m_fd,128,peer_addr);
        m_connection->setConnectionType(TcpConnection::TcpConnectionType::TcpConnectionByClient);
    }

    TcpClient::~TcpClient()
    {
        DEBUGLOG("~TcpClient");
        if(m_fd > 0){
            close(m_fd);
        }
    }

    void TcpClient::connect(std::function<void()> done)
    {
        int rt = ::connect(m_fd,m_peer_addr->getSocketAddr(),m_peer_addr->getSocketLen());
        if (rt == 0){
            DEBUGLOG("connect [%s] success ",m_peer_addr->toString().c_str());
            if(done){
                done();
            }
            m_fd_event->cancle(FdEvent::OUT_EVENT);
            m_event_loop->addEpollEvent(m_fd_event);
        } else if(rt ==-1){
            if(errno == EINPROGRESS){
                m_fd_event ->listen(FdEvent::OUT_EVENT,[this,done](){
                    int error = 0;
                    socklen_t error_len = sizeof(error);
                    getsockopt(m_fd,SOL_SOCKET,SO_ERROR,&error,&error_len);
                    if(error == 0){
                        DEBUGLOG("connect [%s] success ",m_peer_addr->toString().c_str());
                        if(done){
                            done();
                        }   
                    } else{
                        ERRORLOG("connect error, errno = %d, error=%s",errno,strerror(errno));
                    }
                    //链接完成后去掉可写触发
                    m_fd_event->cancle(FdEvent::OUT_EVENT);
                    m_event_loop->addEpollEvent(m_fd_event);
                });

                m_event_loop->addEpollEvent(m_fd_event);
                if(!m_event_loop->isLooping()) m_event_loop->loop();
            } else {
                ERRORLOG("connect error, errno = %d, error=%s",errno,strerror(errno));
            }
        }
    }

    void TcpClient::writeMessage(AbstractProtocal::s_ptr message, std::function<void(AbstractProtocal::s_ptr)> done)
    {
    }

    void TcpClient::readMessage(AbstractProtocal::s_ptr message, std::function<void(AbstractProtocal::s_ptr)> done)
    {
    }

} // namespace rocket

