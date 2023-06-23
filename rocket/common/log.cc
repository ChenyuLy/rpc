// #include "log.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"
#include <sys/time.h>
#include <sstream>
#include <stdio.h>
#include "log.h"
#include "rocket/net/eventloop.h"


namespace rocket
{

    static Logger *g_logger = nullptr;
    Logger *Logger::GetGlobgalLogger()
    {
        return g_logger;
    }

    void Logger::InitGlobgalLogger()
    {
        LogLevel globel_log_level = StringToLoglevel(Config::GetGlobalConfig()->m_log_level);
        g_logger = new Logger(globel_log_level);
        g_logger->init();
        
    }
    std::string LogLevelToString(LogLevel level)
    {
        switch (level)
        {
        case Debug:
            return "Debug";
        case Info:
            return "Info";
        case Error:
            return "Error";
        default:
            return "UNKNOW";
        }
        return std::string();
    }

    LogLevel StringToLoglevel(const std::string &log_level)
    {
        if(log_level == "DEBUG"){
            return Debug;
        } else if(log_level == "INFO"){
            return Info;
        }else if(log_level == "ERROR"){
            return Error;
        }else if(log_level == "UNKNOW"){
            return Unknow;
        }
        return Unknow;
    }

    std::string LogEvent::toString()
    {
        struct timeval now_time;

        gettimeofday(&now_time, nullptr);

        struct tm now_time_t;

        localtime_r(&(now_time.tv_sec), &now_time_t);

        char buf[128];
        strftime(&buf[0], 128, "%y-%m-%d %H:%M:%S", &now_time_t);
        std::string time_str(buf);

        int ms = now_time.tv_usec / 1000;
        time_str = time_str + "." + std::to_string(ms);

        m_thread_id = getThreadId();
        m_pid = getPid();

        std::stringstream ss;

        ss << "[" << LogLevelToString(m_level) << "]\t"
           << "[" << time_str << "]\t"
           << "[" << m_pid << ":"<< m_thread_id << "]\t";
        //    << "[" << std::string(__FILE__) << ":" << __LINE__ << "]\t";

        return ss.str();
    }

    void Logger::log()
    {
        // ScopeMutext<Mutex> lock(m_mutex);
        // std::queue<std::string> tmp ;
        // m_buffer.swap(tmp);
        // // while(!m_buffer.empty()){
        // //     m_buffer.pop();
        // // }
        // lock.unlock();

        // while (!tmp.empty())
        // {
        //     std::string msg = tmp.front();
        //     tmp.pop();

        //     printf("%s",msg.c_str());
        // }
        
    } 

    void Logger::pushLog(const std::string &msg)
    {
        // ScopeMutext<Mutex> lock(m_mutex);
        std::unique_lock<std::mutex> lock(m_mutex);
        m_buffer.push_back(msg);
        lock.unlock();
    }

    void Logger::init()
    {
        m_asnyc_logger =  std::make_shared<AsyncLogger>
                    (Config::GetGlobalConfig()->m_log_file_name,
                    Config::GetGlobalConfig()->m_log_file_path,
                    Config::GetGlobalConfig()->m_log_max_file_size);
        m_timer_event =std::make_shared<TimerEvent>(Config::GetGlobalConfig()->m_log_sync_inteval,true,std::bind(&Logger::syncLoop,this));
        EventLoop::GetCurrentEventLoop()->addTimerEvent(m_timer_event);
    }

    Logger::Logger(LogLevel level):m_set_level(level)
    {

    }

    void Logger::syncLoop()
    {
        //同步m_buffer 到 async_logger 的buffer队列
        std::vector<std::string> tmp_vec = m_buffer;
        std::unique_lock<std::mutex> lock(m_mutex);
        m_buffer.swap(tmp_vec);

        lock.unlock();
        if(!tmp_vec.empty())
        m_asnyc_logger->pushLogBufffer(tmp_vec);



    }

    AsyncLogger::AsyncLogger(std::string &file_name, std::string file_path, int max_size):m_file_name(file_name),m_file_path(file_path),m_max_file_size(max_size)
    {
        sem_init(&m_sempahore,0,0);

        pthread_create(&m_thread,NULL,&AsyncLogger::loop,this);

        // pthread_cond_init(&m_condtion,NULL);

        sem_wait(&m_sempahore);
    }

    void*  AsyncLogger::loop(void* arg)
    {
        AsyncLogger* logger  = reinterpret_cast<AsyncLogger*>(arg);

        sem_post(&logger->m_sempahore);

        printf("333333");
        while(1){
            std::unique_lock<std::mutex>lock(logger->m_mutex);
            while(logger->m_buffer.empty()){
                logger->m_condtion.wait(lock);
            }

            std::vector<std::string> tmp;
            tmp.swap(logger->m_buffer.front());
            logger->m_buffer.pop();

            lock.unlock();

            timeval now;
            gettimeofday(&now,NULL);

            struct tm now_time;
            localtime_r(&(now.tv_sec),&now_time);
            
            const char* format = "%Y%m%d";
            char date[32];
            strftime(date,sizeof(date),format,&now_time);

            if(std::string(date) != logger->m_date){
                logger->m_no = 0;
                logger->m_reopen_flag = true;
                logger->m_date = std::string(date);
            }
            if (logger->m_file_handler == NULL){
                logger->m_reopen_flag = true;
            }

            std::stringstream ss;
            ss<< logger->m_file_path<<logger->m_file_name<<"_"<<std::string(date) << ".";
            std::string log_file_name = ss.str() + std::to_string(logger->m_no);

            if(logger->m_reopen_flag){
                if(logger->m_file_handler){
                    fclose(logger->m_file_handler);
                }
                logger->m_file_handler = fopen(log_file_name.c_str(),"a");
                logger->m_reopen_flag = false;
            }
            printf("22222");
            if(ftell(logger->m_file_handler) > logger->m_max_file_size){
                fclose(logger->m_file_handler);
                printf("1111");
                logger->m_no++;
                log_file_name = ss.str() +std::to_string(logger->m_no++);
                logger->m_file_handler = fopen(log_file_name.c_str(),"a");
                logger->m_reopen_flag = false;
            }

            for(auto & i:tmp){
                if(!i.empty()){
                    fwrite(i.c_str(),1,i.length(),logger->m_file_handler);
                }
            }

            fflush(logger->m_file_handler);    

            if(logger->m_stop_flag){
                return NULL;
            }

        }
        return NULL;
    }
    void AsyncLogger::stop()
    {
        m_stop_flag = true;
    }
    void AsyncLogger::flush()
    {
        if(m_file_handler){
            fflush(m_file_handler);
        }
    }
    void AsyncLogger::pushLogBufffer(std::vector<std::string> &vec)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_buffer.push(vec);
        lock.unlock();
        //这时候需要唤醒异步日志线程
        m_condtion.notify_all();

    }
}
