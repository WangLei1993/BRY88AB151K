#ifndef _sgp30_config_hpp_
#define _sgp30_config_hpp_

#include <string>

#include "base_config.hpp"

namespace cubestone_wang 
{

namespace sensor
{

class SGP30Config: public config::BaseConfig
{
    public:
        // 日志标签
        static const char *const LOG_TAG;
        void Reset();
        std::string Dump();
        bool Load(const char *const config_data);
        bool Load(const std::string &config_data);
        uint16_t CO2eq;
        uint16_t TVOC;
        static const uint16_t Default_CO2eq;
        static const uint16_t Default_TVOC;
};

}

}

#endif // _sgp30_config_hpp_
