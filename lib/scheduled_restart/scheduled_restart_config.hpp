#ifndef _restart_config_hpp_
#define _restart_config_hpp_

#include <string>

#include "scheduled_restart.hpp"

#include "base_config.hpp"

namespace cubestone_wang 
{

namespace scheduled_restart
{

class Config: public config::BaseConfig
{
    public:
        // 日志标签
        static const char *const LOG_TAG;
        void Reset();
        std::string Dump();
        bool Load(const char *const config_data);
        bool Load(const std::string &config_data);
        ScheduledRestart::DayType DayType;
        uint8_t Hour;
        uint8_t Minute;
    private:
        static const ScheduledRestart::DayType default_day_type; 
        static const uint8_t default_hour; 
        static const uint8_t default_minute; 
};

}

}

#endif // _restart_config_hpp_
