#ifndef ROCKET_COMMON_CONFIG_H
#define ROCKET_COMMON_CONFIG_H

// #include <map>
#include <string>

namespace rocket
{
    class Config
    {
    public:
        // std::map<std::string,std::string> m_config_values;
        std::string m_log_level;
        std::string m_log_file_name;
        std::string m_log_file_path;
        int m_log_max_file_size{0};

        int m_log_sync_inteval {0};//日志同步间隔 ms
    public:
        Config(const char* xmlfile);
        static Config* GetGlobalConfig();
        static void SetGlobelConfig(const char* xmlfile);
        

    };
    

    
} // namespace rocket


#endif