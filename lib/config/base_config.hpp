#ifndef _base_config_hpp_
#define _base_config_hpp_

#include <string>

namespace cubestone_wang 
{

namespace config
{

class BaseConfig
{
    public:
        // 日志标签
        static const char *const LOG_TAG;
        virtual void Reset()=0;
        virtual std::string Dump()=0;
        virtual bool Load(const char *const config_data)=0;
        virtual bool Load(const std::string &config_data)=0;
        virtual ~BaseConfig(){};
};

}

}

#endif // _base_config_hpp_
