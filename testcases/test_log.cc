#include "rocket/common/log.h"
#include "rocket/common/config.h"
#include <pthread.h>

void * fun(void *){
    INFOLOG("intfo this is thread in %s","fun")
    DEBUGLOG("debug this is thread in %s","fun");
    return NULL;
}

int main()
{
    rocket::Config::SetGlobelConfig("conf/rocket.xml");
    rocket::Logger::InitGlobgalLogger();
    pthread_t thread;
    pthread_create(&thread,NULL,&fun,NULL);
    DEBUGLOG("test debug log %s  %s","11","22");
    INFOLOG("test info log %s","11");

    pthread_join(thread,NULL);
    return 0;
}
