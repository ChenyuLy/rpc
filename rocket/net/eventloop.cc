#include "rocket/net/eventloop.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include "eventloop.h"
#include <string.h>

#define DEL_TO_EPOLL()                                                                                             \
    auto it = m_listem_fds.find(event->getFd());                                                                   \
    if (it == m_listem_fds.end())                                                                                  \
    {                                                                                                              \
        return;                                                                                                    \
    }                                                                                                              \
    int op = EPOLL_CTL_DEL;                                                                                        \
    epoll_event tmp = event->getEpollEvent();                                                                      \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp);                                                      \
    if (rt == -1)                                                                                                  \
    {                                                                                                              \
        ERRORLOG("failed epoll_ctl when add fd %d , errno=%d,error = %s", event->getFd(), errno, strerror(errno)); \
    }                                                                                                              \
    m_listem_fds.erase(event->getFd());                                                                            \
    DEBUGLOG("delete event success,fd[%d]", event->getFd());

#define ADD_TO_EPOLL()                                                                                             \
    auto it = m_listem_fds.find(event->getFd());                                                                   \
    int op = EPOLL_CTL_ADD;                                                                                        \
    if (it != m_listem_fds.end())                                                                                  \
    {                                                                                                              \
        op = EPOLL_CTL_MOD;                                                                                        \
    }                                                                                                              \
    epoll_event tmp = event->getEpollEvent();                                                                      \
    INFOLOG("epoll_event.events = %d", (int)tmp.events);                                                           \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp);                                                      \
    if (rt == -1)                                                                                                  \
    {                                                                                                              \
        ERRORLOG("failed epoll_ctl when add fd %d , errno=%d,error = %s", event->getFd(), errno, strerror(errno)); \
    }                                                                                                              \
    m_listem_fds.insert(event->getFd());                                                                           \
    DEBUGLOG("add event success,fd[%d]", event->getFd());

namespace rocket
{

    static thread_local EventLoop *t_current_eventloop = NULL;
    static int g_epoll_max_timeout = 10000;
    static int g_epoll_max_events = 10;

    EventLoop::EventLoop()
    {
        if (t_current_eventloop != NULL)
        {
            ERRORLOG("failed to create event loop,  this thread has created event loop");
            exit(0);
        }
        m_thread_id = getThreadId();
        m_epoll_fd = epoll_create(10);
        if (m_epoll_fd == -1)
        {
            ERRORLOG("failed to create event loop,  epoll_create rooro , error info[%S]", errno);
            exit(0);
        }

        initWakeUpFdEvent();
        initTimer();
        INFOLOG("succ create event loop in thread % d", m_thread_id);
        t_current_eventloop = this;
        // DEBUGLOG("2222222222222222222222");
    }

    EventLoop::~EventLoop()
    {
        close(m_epoll_fd);
        if (m_wakeup_fd_event)
        {
            delete m_wakeup_fd_event;
            m_wakeup_fd_event = nullptr;
        }
        if (m_timer)
        {
            delete m_timer;
            m_timer = NULL;
        }
    }

    void EventLoop::loop()
    {
        m_is_looping =true;
        while (!m_stop_flag)
        {
            // ScopeMutext<Mutex> lock(m_mutex);
            rocket::SafeQueue<std::function<void()>> tmp_tasks;
            m_pending_tasks.swap(tmp_tasks);
            // lock.unlock();

            while (!tmp_tasks.empty())
            {
                std::function<void()> cb = tmp_tasks.front();
                tmp_tasks.pop();
                if (cb)
                {
                    cb();
                }
            }
            // 如果有定时任务需要执行那么执行
            // 1 怎么判断定时人物需要执行？ now -> TinmeEvent.arrtive_time
            // 2.arrtive_time r如何监听 eventloop 监听
            //

            int timeout = g_epoll_max_timeout;
            epoll_event result_events[g_epoll_max_events];

            DEBUGLOG("now begin to epoll_wait");
            int rt = epoll_wait(m_epoll_fd, result_events, g_epoll_max_events, timeout);
            DEBUGLOG("now end to epoll_wait,rt = %d", rt);
            if (rt < 0)
            {
                ERRORLOG("epoll_wait error,errno=%d", errno);
            }
            else
            {
                for (int i = 0; i < rt; i++)
                {
                    epoll_event trigger_event = result_events[i];
                    FdEvent *fd_event = static_cast<FdEvent *>(trigger_event.data.ptr);
                    if (fd_event == NULL)
                        continue;
                    if (trigger_event.events & EPOLLIN)
                    {
                        DEBUGLOG("fd %d trigger EPOLLIN event", fd_event->getFd());
                        addTask(fd_event->handler(FdEvent::IN_EVENT));
                    }
                    if (trigger_event.events & EPOLLOUT)
                    {
                        DEBUGLOG("fd %d trigger EPOLLOUT event", fd_event->getFd());
                        addTask(fd_event->handler(FdEvent::OUT_EVENT));
                    }
                    // if(!(trigger_event.events & EPOLLIN) && (trigger_event.events & EPOLLOUT)){
                    //     int evbent = (int)(trigger_event.events);
                    //     DEBUGLOG("Unknow event = %s",evbent);
                    // }
                    if(trigger_event.events & EPOLLERR){
                        DEBUGLOG("fd %d trigger EPOLLERR event", fd_event->getFd());
                        deleteEpollEvent(fd_event);

                        if(fd_event -> handler(FdEvent::ERR_EVENT) != nullptr){
                            DEBUGLOG("fd %d add error callback", fd_event->getFd());
                            addTask(fd_event->handler(FdEvent::OUT_EVENT));
                        }
                    }
                }
            }
        }
    }

    void EventLoop::wakeup()
    {
        m_wakeup_fd_event->wakeup();
    }

    void EventLoop::stop()
    {
        m_stop_flag = true;
        wakeup();
    }

    void EventLoop::addEpollEvent(FdEvent *event)
    {
        if (isInLoopThread())
        {
            ADD_TO_EPOLL();
        }
        else
        {
            auto cb = [this, event]()
            {
                DEBUGLOG("in else");
                ADD_TO_EPOLL();
            };
            addTask(cb, true);
        }
        // ADD_TO_EPOLL();
    }
    void EventLoop::deleteEpollEvent(FdEvent *event)
    {
        if (isInLoopThread())
        {
            DEL_TO_EPOLL();
        }
        else
        {
            auto cb = [this, event]()
            {
                DEL_TO_EPOLL();
            };
            addTask(cb, true);
        }
    }
    bool EventLoop::isInLoopThread()
    {
        return getThreadId() == m_thread_id;
    }
    void EventLoop::addTask(std::function<void()> cb, bool is_wake_up)
    {
        // DEBUGLOG("befor push 1");
        // ScopeMutext<Mutex> lock(m_mutex);
        // DEBUGLOG("befor push 2");
        m_pending_tasks.push(cb);
        // lock.unlock();

        // DEBUGLOG("after push ");
        if (is_wake_up)
        {
            DEBUGLOG("wakeup");
            wakeup();
        }
        // DEBUGLOG("after wakeup");
    }
    void EventLoop::addTimerEvent(TimerEvent::s_ptr event)
    {
        m_timer->addTimerEvent(event);
    }
    EventLoop *EventLoop::GetCurrentEventLoop()
    {
        if (t_current_eventloop)
        {
            return t_current_eventloop;
        }
        t_current_eventloop = new EventLoop();
        return t_current_eventloop;
    }
    bool EventLoop::isLooping()
    {
        return m_is_looping;
    }
    void EventLoop::dealWakeup()
    {
    }

    void EventLoop::initWakeUpFdEvent()
    {
        m_wakeup_fd = eventfd(0, EFD_NONBLOCK);
        if (m_wakeup_fd < 0)
        {
            ERRORLOG("failed to create event loop,  epoll_create rooro , error info[%S]", errno);
            exit(0);
        }
        m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);
        m_wakeup_fd_event->listen(FdEvent::IN_EVENT, [this]()
                                  {
            char buf[8];
            while (read(m_wakeup_fd, buf, 8) != -1 && errno == EAGAIN){};
            DEBUGLOG("read full bytes from wakeup fd[%d]", m_wakeup_fd); });
        addEpollEvent(m_wakeup_fd_event);
    }

    void EventLoop::initTimer()
    {
        m_timer = new Timer();
        addEpollEvent(m_timer);
    }

} // namespace rocket
