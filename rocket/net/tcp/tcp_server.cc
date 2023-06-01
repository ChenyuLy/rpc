#include "tcp_server.h"

rocket::TCPServer::TCPServer(NetAddr::s_ptr local_addr) : m_local_addr(local_addr)
{
    init();
    INFOLOG("rocket TcpServer listen succes on [%s]", m_local_addr->toString().c_str());
}

rocket::TCPServer::~TCPServer()
{
    if (m_main_event_loop)
    {
        delete m_main_event_loop;
        m_main_event_loop = NULL;
    }
    if (listen_fd_event)
    {
        delete listen_fd_event;
        listen_fd_event = NULL;
    }
}

void rocket::TCPServer::start()
{
    m_io_thread_group->start();
    m_main_event_loop->loop();
}

void rocket::TCPServer::init()
{
    m_acceptor = std::make_shared<TcpAcceptor>(m_local_addr);

    m_main_event_loop = EventLoop::GetCurrentEventLoop();
    m_io_thread_group = new IOThreadGroup(2);

    listen_fd_event = new FdEvent(m_acceptor->getListenFd());
    listen_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TCPServer::onAccept, this));
    
    m_main_event_loop->addEpollEvent(listen_fd_event);
}

void rocket::TCPServer::onAccept()
{
    auto re = m_acceptor->accept();
    int client_fd = re.first;
    NetAddr::s_ptr peer_addr = re.second;
    if(client_fd > 0) m_client_counts++;
    //TODO:把client—fd添加到IO线程里面
    IOThread* io_thread =m_io_thread_group->getIOThread();
    TcpConnection::s_ptr connection = std::make_shared<TcpConnection> (io_thread->getEventLoop(),client_fd,128,peer_addr);

    connection->setState(TcpConnection::TcpState::Connected);
    m_client.insert(connection);
    INFOLOG("TCPServer succ get client fd=%d,",client_fd);

}
