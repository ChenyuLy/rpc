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
    public:
        Config(const char* xmlfile);
        static Config* GetGlobalConfig();
        static void SetGlobelConfig(const char* xmlfile);
        
    };
    

    
} // namespace rocket


#endif