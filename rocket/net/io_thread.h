#ifndef ROCKET_NET_IO_THREAD_H
#define ROCKET_NET_IO_THREAD_H


#include <pthread.h>
#include <semaphore.h>
#include <rocket/net/eventloop.h>

namespace rocket{

class IOThread
{
private:
    pid_t m_thread_id {-1}; //进程id
    pthread_t m_thread{0};//线程句柄
    EventLoop* m_event_loop {NULL} ;//当前对象的loop对象

    sem_t m_init_semaphore;
    sem_t m_start_semaphore;

    

public:
    IOThread();
    ~IOThread();

    EventLoop* getEventLoop();
    void start();

    void join();

    static void * Main(void * arg);
};


}


#endif