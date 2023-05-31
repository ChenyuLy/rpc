#ifndef ROCKET_NET_EVENT_GROUP_H
#define ROCKET_NET_EVENT_GROUP_H


#include <vector>
#include "rocket/net/fd_event.h"
#include <mutex>

namespace rocket
{
    class FdEventGroup
    {
    private:
        int m_size {0};
        std::vector<FdEvent*> m_fd_group;
        std::mutex m_mutex;

    public:
        FdEventGroup(int size);

        FdEvent* getFdEvent(int fd);
        ~FdEventGroup();


        static FdEventGroup* GetFdEventGroup();
    };
    

    
} // namespace rocket


#endif