#ifndef ROCKET_COMMON_LOG_H
#define ROCKET_COMMON_LOG_H

#include <string.h>
#include <queue>
#include <memory>
#include<semaphore.h>
#include <condition_variable>
#include <mutex>
// #include <common/config.h>
#include "rocket/common/config.h"
#include "rocket/common/mutex.h"
#include "rocket/net/timer_event.h"

namespace rocket
{

    template <typename... Args>
    std::string formatString(const char *str, Args &&...args)
    {
        int size = snprintf(nullptr, 0, str, args...);
        // int size = strlen(str);
        std::string result;
        if (size > 0)
        {
            result.resize(size);
            snprintf(&result[0], size + 1, str, args...);
            // memcpy(&result,)
            // result = str;

        }

        return std::string(result);
    }

#define DEBUGLOG(str, ...)                                                                                                                                                                                                 \
    if (rocket::Logger::GetGlobgalLogger()->getLogLevel() <= rocket::Debug)                                                                                                                                                \
    {                                                                                                                                                                                                                      \
        rocket::Logger::GetGlobgalLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Debug).toString() + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n"); \
    }

#define INFOLOG(str, ...)                                                                                                                                                                                                 \
    if (rocket::Logger::GetGlobgalLogger()->getLogLevel() <= rocket::Info)                                                                                                                                                \
    {                                                                                                                                                                                                                     \
        rocket::Logger::GetGlobgalLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Info).toString() + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n"); \
    }

#define ERRORLOG(str, ...)                                                                                                                                                                                                 \
    if (rocket::Logger::GetGlobgalLogger()->getLogLevel() <= rocket::Error)                                                                                                                                                \
    {                                                                                                                                                                                                                      \
        rocket::Logger::GetGlobgalLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Error).toString() + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n"); \
    }

    enum LogLevel
    {
        Unknow = 0,
        Debug = 1,
        Info = 2,
        Error = 3
    };

    std::string LogLevelToString(LogLevel level);
    LogLevel StringToLoglevel(const std::string &log_level);


    class AsyncLogger
    {
    private:
        // m_file_path/m_file_name_yyyymmdd.m_no
        std::string m_file_name; //日志输出的名字
        std::string m_file_path; //日志和i输出路径
        int m_max_file_size {0}; //

        std::queue<std::vector<std::string>> m_buffer;

        sem_t m_sempahore;
        pthread_t m_thread;

        // pthread_cond_t m_condtion; //tiaoji bianliang
        std::condition_variable m_condtion;

        std::mutex m_mutex;

        std::string m_date; //当前答应的日期
        FILE* m_file_handler{NULL}; //当前打开日志文件的句柄

        bool m_reopen_flag {false};

        int m_no{0};//日志文件序号

        bool m_stop_flag{false};

    public:
        typedef std::shared_ptr<AsyncLogger> s_ptr;
        AsyncLogger(std::string& file_name,std::string file_path,int max_size);
        // ~AsyncLogger();
        
        static void* loop(void* );
        void stop();

        void flush();

        void pushLogBufffer(std::vector<std::string>& vec);
    };
    
    class Logger
    {
    private:
        LogLevel m_set_level;
        std::vector<std::string> m_buffer;


        std::mutex m_mutex;
        AsyncLogger::s_ptr m_asnyc_logger;
    // m_file_path/m_file_name_yyyymmdd.m_no
        std::string m_file_name; //日志输出的名字
        std::string m_file_path; //日志和i输出路径
        int m_max_file_size {0}; //

        TimerEvent::s_ptr m_timer_event;

    public:
        static void InitGlobgalLogger();
        static Logger *GetGlobgalLogger();

    public:
        void log();
        void pushLog(const std::string &msg);
        void init();
        typedef std::shared_ptr<Logger> s_ptr;

        Logger(LogLevel level);
        LogLevel getLogLevel() const
        {
            return m_set_level;
        }

        void syncLoop();
    };

    class LogEvent
    {

    private:
        std::string m_file_name; // 文件名
        int32_t m_file_line;     // 行号
        int32_t m_pid;
        int32_t m_thread_id;
        LogLevel m_level;



    public:
        std::string getFileName() const
        {
            return m_file_name;
        }

        LogLevel getLogLevel() const
        {
            return m_level;
        }

        std::string toString();

        LogEvent(LogLevel level) : m_level(level){};
    };

    // void DEBUGLOG(std::string str,...);










}












#endif