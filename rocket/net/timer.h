#ifndef ROCKET_NET_TIMER_H
#define ROCKET_NET_TIMER_H
#include <map>
#include "rocket/net/fd_event.h"
#include "rocket/net/timer_event.h"
// #include "rocket/common/mutex.h"
#include <mutex>

namespace rocket
{
    class Timer : public FdEvent
    {
    private:
        std::multimap<int64_t, TimerEvent::s_ptr> m_pending_events;
        // Mutex m_mutex;
        std::mutex m_mutex;

        void resetArriveTime();
        
    public:
        Timer ();
        ~Timer ();

        void addTimerEvent(TimerEvent::s_ptr event);
        void deleteTimerEvent(TimerEvent::s_ptr event);

        void onTimer(); // 当发生了io事件以后eventloop 会执行这个会执行这个函数
        
    };
    

    
} // namespace rocket





#endif
