#ifndef ROCKET_COMMON_RUN_TIME_H
#define ROCKET_COMMON_RUN_TIME_H

#include <string>

namespace rocket
{   
    class RunTime
    {
    private:
        
    public:
        // RunTime();
        // ~RunTime();

        static RunTime* GetRunTime();
        
        std::string m_method_name;
        std::string m_msgid;

    };
        
} // namespace rocket


#endif