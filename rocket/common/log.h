#ifndef ROCKET_COMMON_LOG_H
#define ROCKET_COMMON_LOG_H

#include <string.h>
#include <queue>
#include <memory>
// #include <common/config.h>
#include "rocket/common/config.h"
#include "rocket/common/mutex.h"

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
        rocket::Logger::GetGlobgalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Debug))->toString() + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n"); \
        rocket::Logger::GetGlobgalLogger()->log();                                                                                                                                                                         \
    }

#define INFOLOG(str, ...)                                                                                                                                                                                                 \
    if (rocket::Logger::GetGlobgalLogger()->getLogLevel() <= rocket::Info)                                                                                                                                                \
    {                                                                                                                                                                                                                     \
        rocket::Logger::GetGlobgalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Info))->toString() + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n"); \
        rocket::Logger::GetGlobgalLogger()->log();                                                                                                                                                                        \
    }

#define ERRORLOG(str, ...)                                                                                                                                                                                                 \
    if (rocket::Logger::GetGlobgalLogger()->getLogLevel() <= rocket::Error)                                                                                                                                                \
    {                                                                                                                                                                                                                      \
        rocket::Logger::GetGlobgalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Error))->toString() + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n"); \
        rocket::Logger::GetGlobgalLogger()->log();                                                                                                                                                                         \
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

    class Logger
    {
    private:
        LogLevel m_set_level;
        std::queue<std::string> m_buffer;

        Mutex m_mutex;

    public:
        static void InitGlobgalLogger();
        static Logger *GetGlobgalLogger();

    public:
        void log();
        void pushLog(const std::string &msg);
        typedef std::shared_ptr<Logger> s_ptr;

        Logger(LogLevel level) : m_set_level(level){};
        LogLevel getLogLevel() const
        {
            return m_set_level;
        }
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