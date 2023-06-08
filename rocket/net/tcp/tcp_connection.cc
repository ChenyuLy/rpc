#include "tcp_connection.h"
#include "rocket/net/string_coder.h"
namespace rocket
{

    TcpConnection::TcpConnection(EventLoop *event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr, TcpConnectionType type)
        : m_event_loop(event_loop), m_fd(fd), m_peer_addr(peer_addr), m_stat(NotConnected), m_connection_type(type)
    {
        m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
        m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

        m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
        m_fd_event->setNonBlock();

        if (m_connection_type == TcpConnectionByServer)
        {
            listenRead();
        }

        m_coder = new StringCoder();
    }

    TcpConnection::~TcpConnection()
    {
        DEBUGLOG("~TcpConnection");
        if (m_coder)
        {
            delete m_coder;
            m_coder = NULL;
        }
    }

    void TcpConnection::setConnectionType(TcpConnectionType type)
    {
        m_connection_type = type;
    }

    void TcpConnection::onRead()
    {
        // 1.从socket缓冲区中调用系统的read函数读取字节 到inbuffer利民啊
        if (m_stat != Connected)
        {
            ERRORLOG("OnRead  error client has already disconnected,addr[%s],clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
            return;
        }

        bool is_read_all = false;
        bool is_close = false;
        while (!is_read_all)
        {
            if (m_in_buffer->writeAble() == 0)
                m_in_buffer->resizeBuffer(2 * m_in_buffer->m_buffer.size());
            int read_count = m_in_buffer->writeAble();
            int write_index = m_in_buffer->writeIndex();

            int rt = read(m_fd, &(m_in_buffer->m_buffer[write_index]), read_count);
            DEBUGLOG("succ read %d , bytes from addr [%s],client fd[%d]", rt, m_peer_addr->toString().c_str(), m_fd);
            if (rt > 0)
            {
                m_in_buffer->moveWriteIndex(rt);
                if (rt == read_count)
                {
                    continue;
                }
                else if (rt < read_count)
                {
                    is_read_all = true;
                    break;
                }
            }
            else if (rt == 0)
            {
                is_close = true;
                break;
            }
            else if (rt == -1 && errno == EAGAIN)
            {
                is_read_all = true;
                break;
            }
        }

        if (is_close)
        {
            // 处理关闭链接

            DEBUGLOG("peer closed,peer addr [%s],client fd[%d] ", m_peer_addr->toString().c_str(), m_fd);
            clear();
            return;
        }

        if (!is_read_all)
        {
            ERRORLOG("not read all data");
        }
        // TODO: 简单的echo,后面补充RPC协议解析
        excute();
    }

    void TcpConnection::excute()
    {

        if (m_connection_type == TcpConnectionByServer)
        {
            // 将RPC请求执行业务逻辑，获取RPC响应，再把RPC响应发送回去
            std::vector<char> tmp;
            int size = m_in_buffer->readAble();
            tmp.resize(size);
            m_in_buffer->readFromBuffer(tmp, size);

            std::string msg;
            for (size_t i = 0; i < tmp.size(); ++i)
            {
                msg += tmp[i];
            }

            INFOLOG("succ get request[%s] from client[%s]", msg.c_str(), m_peer_addr->toString().c_str());

            m_out_buffer->writeToBuffer(msg.c_str(), msg.length());
            listenWrite();
        } else {
            //2从buffer里decode得到message 对象 执行回调
            std::vector<AbstractProtocal::s_ptr> result;
            m_coder->decode(result,m_in_buffer);

            for(size_t i= 0;i<result.size();++i){
                std::string req_id = result[i]->getReqID();
                auto it = m_read_dones.find(req_id);
                if(it != m_read_dones.end()){
                    it->second(result[i]->shared_from_this());
                }
            }

        }
        
    }

    void TcpConnection::onWrite()
    {
        // 将当前缓冲区oubuff的数去全部发送到client

        if (m_stat != Connected)
        {
            ERRORLOG("onWrite error client has already disconnected,addr[%s],clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
        }

        if (m_connection_type == TcpConnectionByClient)
        {
            // 将message encode得到字节流
            // 将字节流写入到buffer里面,全部发送
            std::vector<AbstractProtocal ::s_ptr> messages;

            for (size_t i = 0; i < m_write_dones.size(); ++i)
            {
                messages.push_back(m_write_dones[i].first);
            }
            m_coder->encode(messages, m_out_buffer);
        }

        bool is_write_all = false;
        while (true)
        {
            if (m_out_buffer->readAble() == 0)
            {
                DEBUGLOG("no data need to send to client[%s]", m_peer_addr->toString().c_str());
                is_write_all = true;
                break;
            }
            int write_size = m_out_buffer->readAble();
            int read_index = m_out_buffer->readIndex();
            int rt = write(m_fd, &(m_out_buffer->m_buffer[read_index]), write_size);

            if (rt >= write_size)
            {
                DEBUGLOG("no data need to send to client[%s]", m_peer_addr->toString().c_str());
                is_write_all = true;
                break;
            }
            if (rt == -1 && errno == EAGAIN)
            {
                // 发送区已经满了不能发送了
                // 这种情况我们等下次fd可写的时候再次发送数据即可
                ERRORLOG("write data error , rt == -1 && errno == EAGAIN");
                break;
            }
        }
        if (is_write_all)
        {
            m_fd_event->cancle(FdEvent::OUT_EVENT);
            // m_io_thread->getEventLoop()->deleteEpollEvent(m_fd_event);
            m_event_loop->addEpollEvent(m_fd_event);
        }

        if (m_connection_type == TcpConnectionByClient)
        {
            for (size_t i = 0; i < m_write_dones.size(); ++i)
            {
                m_write_dones[i].second(m_write_dones[i].first);
            }
            m_write_dones.clear();
        }
    }

    void TcpConnection::shutdown()
    {
        if (m_stat == Closed || m_stat == NotConnected)
        {
            return;
        }
        m_stat = HalfClosing;
        // 关闭读写，意味着 服务器关闭本端操作
        // 发丝不过FIN报文，出发了四次回首的第一个阶段
        // 当fd发生可读时间但是可读的数据为0 即多余的哑巴发送了FIN
        ::shutdown(m_fd, SHUT_RDWR);
    }

    void TcpConnection::listenWrite()
    {
        m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite, this));
        m_event_loop->addEpollEvent(m_fd_event);
    }

    void TcpConnection::listenRead()
    {
        m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead, this));
        m_event_loop->addEpollEvent(m_fd_event);
    }

    void TcpConnection::pushSendMessage(AbstractProtocal::s_ptr message, std::function<void(AbstractProtocal::s_ptr)> done)
    {
        m_write_dones.push_back(std::make_pair(message, done));
    }

    void TcpConnection::pushReadMessage(const std::string &req_id, std::function<void(AbstractProtocal::s_ptr)> done)
    {
        m_read_dones.insert(std::make_pair(req_id,done));
    }

    void TcpConnection::setState(const TcpConnection::TcpState state)
    {
        m_stat = Connected;
    }

    void TcpConnection::clear()
    {
        // 处理一些关闭链接后的清理工作
        if (m_stat == Closed)
        {
            return;
        }
        m_fd_event->cancle(FdEvent::IN_EVENT);
        m_fd_event->cancle(FdEvent::OUT_EVENT);
        m_event_loop->deleteEpollEvent(m_fd_event);

        m_stat = Closed;
    }

    TcpConnection::TcpState TcpConnection::getState()
    {
        return m_stat;
    }

} // namespace rocket
