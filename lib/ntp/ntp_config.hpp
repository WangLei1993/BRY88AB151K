#ifndef _ntp_config_hpp_
#define _ntp_config_hpp_

#include <string>
#include <vector>

#include "base_config.hpp"

namespace cubestone_wang 
{

namespace ntp
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
        std::vector<std::string> ServerNameList;
};

}

}

#endif // _ntp_config_hpp_
