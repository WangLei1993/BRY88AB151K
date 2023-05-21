#ifndef _config_manager_hpp_
#define _config_manager_hpp_

#include <map>
#include <set>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "base_config.hpp"

namespace cubestone_wang 
{

namespace config
{

class ConfigManager
{
    public:
        // 日志标签
        static const char *const LOG_TAG;
        static bool Reset(const std::string &name="");
        static bool Save(const std::string &name="");
        static bool Load(const std::string &name="");
        static BaseConfig *Get(const std::string &name);
        static bool Add(const std::string &name, BaseConfig *config, const bool &override=true);
        static bool Del(const std::string &name);
    private:
        static SemaphoreHandle_t mutex;
        static const char *const ns;
        static std::map<std::string, BaseConfig *> config_map;
        typedef bool (*InnerFunc_t)(const std::string &name, bool debug_log_enable);
        typedef void (*EndFunc_t)();
        static bool inner_reset(const std::string &name="", bool debug_log_enable=true);
        static bool inner_save(const std::string &name="", bool debug_log_enable=true);
        static bool inner_load(const std::string &name="", bool debug_log_enable=true);
        static bool inner_run_func(InnerFunc_t inner_func, EndFunc_t end_func, const std::string &name="");
};

}

}

#endif // _config_manager_hpp_
