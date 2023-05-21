#ifndef _mdns_config_hpp_
#define _mdns_config_hpp_

#include <string>

#include "mdns.hpp"

#include "base_config.hpp"

namespace cubestone_wang 
{

namespace mdns
{

class Config: public config::BaseConfig
{
    public:
        // 日志标签
        static const char *const LOG_TAG;
        Config();
        virtual ~Config();
        void Reset();
        std::string Dump();
        bool Load(const char *const config_data);
        bool Load(const std::string &config_data);
        std::string Hostname;
        std::string InstanceName;
        std::vector<MDNS::Service> services;
    private:
        char *default_hostname;
        static const char *const default_instance_name;
        std::vector<MDNS::Service> default_services;
};

}

}

#endif // _mdns_config_hpp_
