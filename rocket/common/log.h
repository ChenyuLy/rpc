#ifndef ROCKET_COMMON_LOG_H
#define ROCKET_COMMON_LOG_H

#include <string>
#include <vector>
#include <queue>
#include <memory>

namespace rocket
{

    template <typename... Args>
    std::string formatString(const char *str, Args &&...args)
    {
        int size = snprintf(nullptr, 0, str, args...);

        std::string result;
        if (size > 0)
        {
            result.resize(size);
            snprintf(&result[0], size + 1, str, args...);
        }

        return std::string(result);
    }
#define DEBUGLOG(str, ...)                                                                                                    \
    std::string msg = (new rocket::LogEvent(rocket::LogLevel::Debug))->toString() + rocket::formatString(str, ##__VA_ARGS__) + "\n"; \
    rocket::Logger::GetGlobgalLogger()->pushLog(msg);                                                                           \
    rocket::Logger::GetGlobgalLogger()->log();

    enum LogLevel
    {
        Debug = 1,
        Info = 2,
        Error = 3
    };

    std::string LogLevelToString(LogLevel level);

    class Logger
    {
    private:
        LogLevel m_set_level;
        std::queue<std::string> m_buffer;

    public:
        static Logger* GetGlobgalLogger();

    public:
        void log();
        void pushLog(const std::string &msg);
        typedef std::shared_ptr<Logger> s_ptr;
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