#include "tcp_client.h"
#include <string.h>
#include "rocket/common/errcode.h"
#include "rocket/net/tcp/net_addr.h"
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
            initLocalAddr();
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
                    int rt = ::connect(m_fd,m_peer_addr->getSocketAddr(),m_peer_addr->getSocketLen());
                    if ((rt <0 && errno ==EISCONN ) || (rt == 0)){
                    DEBUGLOG("connect [%s] success 1",m_peer_addr->toString().c_str());
                    initLocalAddr();
                    m_connection-> setState(TcpConnection::TcpState::Connected);
                    } else {
                        if(errno == ECONNREFUSED){
                            this->m_connect_errcode = ERROR_PEER_CLOSED;
                            this->m_connect_error_info = "connect refused , sys errno = " + std::string(strerror(errno)); 
                        } else {
                            this->m_connect_errcode = ERROR_FAILED_CONNECT;
                            this->m_connect_error_info = "connect unkonwn , sys errno = " + std::string(strerror(errno));
                        }

                        ERRORLOG("connect error, errno = %d, error=%s",errno,strerror(errno));
                        close(m_fd);
                        m_fd = socket(m_peer_addr->intGetFamily(), SOCK_STREAM, 0);
                    }
        

                    // int error = 0;
                    // socklen_t error_len = sizeof(error);
                    // getsockopt(m_fd,SOL_SOCKET,SO_ERROR,&error,&error_len);
                    // // bool is_connect_succ = false;
                    // if(error == 0){
                    //     DEBUGLOG("connect [%s] success 1",m_peer_addr->toString().c_str());
                    //     initLocalAddr();
                    //     // is_connect_succ = true;
                    //     m_connection->setState(TcpConnection::TcpState::Connected);

                    // } 
                    // // else if(errno == EINPROGRESS){
                    // //     DEBUGLOG("connect [%s] in progress",m_peer_addr->toString().c_str());
                    // //     return;
                    // // }
                    // else
                    // {
                    //     this->m_connect_errcode = ERROR_FAILED_CONNECT;
                    //     this->m_connect_error_info = "connect error , sys errno = " + std::string(strerror(errno));
                    //     ERRORLOG("connect error, errno = %d, error=%s",errno,strerror(errno));
                    // }
                    //链接完成后去掉可写触发
                    // m_fd_event->cancle(FdEvent::OUT_EVENT);
                    // m_event_loop->addEpollEvent(m_fd_event);
                    m_event_loop->deleteEpollEvent(m_fd_event);
                    //如果链接成功才会执行回调函数
                    if(done){
                            
                            done();
                    } 
                },[this,done](){
                    m_fd_event->cancle(FdEvent::OUT_EVENT);
                    m_event_loop->addEpollEvent(m_fd_event);
                    if(errno == ECONNREFUSED){
                        this->m_connect_errcode = ERROR_FAILED_CONNECT;
                        this->m_connect_error_info = "connect refused , sys errno = " + std::string(strerror(errno));
                    } else{
                        this->m_connect_errcode = ERROR_FAILED_CONNECT;
                        this->m_connect_error_info = "connect unkonw error , sys errno = " + std::string(strerror(errno));
                    }
                    ERRORLOG("connect error, errno = %d, error=%s",errno,strerror(errno));
                });

                m_event_loop->addEpollEvent(m_fd_event);
                if (!m_event_loop->isLooping())
                    m_event_loop->loop();
            }
            else
            {
                ERRORLOG("connect error, errno = %d, error=%s", errno, strerror(errno));
                this->m_connect_errcode = ERROR_FAILED_CONNECT;
                this->m_connect_error_info = "connect error , sys errno = " + std::string(strerror(errno));
                if(done){
                    done();
                }
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

    void TcpClient::readMessage(const std::string& msg_id , std::function<void(AbstractProtocal::s_ptr)> done)
    {
        //1监听可读事件
        //2从buffer里decode得到message 对象 判断是否热情——id相等，相等则成功，执行回调
        m_connection->pushReadMessage(msg_id,done);
        m_connection->listenRead();
    }

    void TcpClient::stop()
    {
        if(m_event_loop->isLooping()){
            m_event_loop->stop();
        }
    }

    int TcpClient::getConnectErrorCode()
    {
        return m_connect_errcode;
    }

    std::string TcpClient::getConnectErrorInfo()
    {
        return m_connect_error_info;
    }

    NetAddr::s_ptr TcpClient::getPeerAddr()
    {
        return  m_peer_addr;
    }

    NetAddr::s_ptr TcpClient::getLocalAddr()
    {
        return m_local_addr;
    }

    void TcpClient::initLocalAddr()
    {
        sockaddr_in local_addr;
        socklen_t len = sizeof(local_addr);

        int ret = getsockname(m_fd,reinterpret_cast<sockaddr*>(&local_addr),&len);
        if(ret != 0) {
            ERRORLOG("initLocalAddr error , getsockname error . errno = %d, error = %s",errno,strerror(errno));
        }

        m_local_addr = std::make_shared<IPNetAddr>(local_addr);
    }

    void TcpClient::addTimerEvent(TimerEvent::s_ptr timer_event)
    {  
        m_event_loop->addTimerEvent(timer_event);
    }

} // namespace rocket
