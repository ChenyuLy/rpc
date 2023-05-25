// #include "log.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"
#include <sys/time.h>
#include <sstream>
#include <stdio.h>
#include "log.h"


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
           << "[" << m_pid << ":"<< m_thread_id << "]\t"
           << "[" << std::string(__FILE__) << ":" << __LINE__ << "]\t";

        return ss.str();
    }

    void Logger::log()
    {
        ScopeMutext<Mutex> lock(m_mutex);
        std::queue<std::string> tmp = m_buffer;
        // m_buffer.swap(tmp);
        while(!m_buffer.empty()){
            m_buffer.pop();
        }
        lock.unlock();

        while (!tmp.empty())
        {
            std::string msg = tmp.front();
            tmp.pop();

            printf("%s",msg.c_str());
        }
        
    } 

    void Logger::pushLog(const std::string &msg)
    {
        ScopeMutext<Mutex> lock(m_mutex);
        m_buffer.push(msg);
        lock.unlock();
    }

}
