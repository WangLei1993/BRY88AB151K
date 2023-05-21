#ifndef _wifi_config_hpp_
#define _wifi_config_hpp_

#include <string>

#include "esp_wifi.h"

#include "base_config.hpp"

namespace cubestone_wang 
{

namespace wifi
{

// TODO: 目前仅支持WPA2方式
class Config: public config::BaseConfig
{
    public:
        struct 
        {
            std::string SSID;
            std::string Password;
            std::string Hostname;
        } AP, STA;
        wifi_mode_t Mode;
        // 日志标签
        static const char *const LOG_TAG;
        Config();
        virtual ~Config();
        void Reset();
        std::string Dump();
        bool Load(const char *const config_data);
        bool Load(const std::string &config_data);
    private:
        char *default_ap_ssid;
        char *default_ap_hostname;
        char *default_sta_hostname;
        static const char *const default_ap_password;
        static const char *const default_sta_ssid;
        static const char *const default_sta_password;
};

}

}

#endif // _wifi_config_hpp_
