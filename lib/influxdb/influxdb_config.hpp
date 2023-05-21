#ifndef _influxdb_config_hpp_
#define _influxdb_config_hpp_

#include <string>

#include "base_config.hpp"

namespace cubestone_wang 
{

namespace influxdb
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
        std::string Host;
        uint16_t Port;
        std::string Token;
        std::string Org;
        std::string Bucket;
        uint8_t Timeout;
    private:
        static const uint16_t default_port;
        static const std::string default_org;
        static const std::string default_bucket;
        static const uint8_t default_timeout;
};

}

}

#endif // _influxdb_config_hpp_
