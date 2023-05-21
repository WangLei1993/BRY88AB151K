#ifndef _application_hpp_
#define _application_hpp_

#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "influxdb.hpp"
#include "i2c_master.hpp"
#include "cm1106.hpp"
#include "hdc1080.hpp"
#include "pm2005.hpp"
#include "sgp30.hpp"
#include "ze08_ch2o.hpp"

namespace cubestone_wang 
{

namespace application
{

class Application
{   
    private:
        static bool start_flag;
        static SemaphoreHandle_t mutex;
        static std::string wifi_monochrome_led_name;
        static std::string pm25_monochrome_led_name;
        static std::string co2_monochrome_led_name;
        static std::string tvoc_monochrome_led_name;
        static std::string inner_monochrome_led_name;
        static std::string sgp30_config_name;
        static std::string scheduled_restart_config_name;
        static std::string influxdb_config_name;
        static std::string ntp_config_name;
        static std::string mdns_config_name;
        static std::string wifi_config_name;
        static i2c_master::I2cMaster *i2c_master_0;
        static i2c_master::I2cMaster *i2c_master_1;
        static sensor::CM1106 *cm1106;
        static sensor::HDC1080 *hdc1080;
        static sensor::PM2005 *pm2005;
        static sensor::SGP30 *sgp30;
        static sensor::ZE08_CH2O *ze08_ch2o;
        static influxdb::Influxdb *influxdb;
        static QueueHandle_t influxdb_queue;
        static bool init();
        static bool init_log();
        static bool init_button();
        static bool init_config();
        static bool init_monochrome_led();
        static bool init_i2c();
        static bool init_uart();
        static bool init_influxdb();
        static void reboot_for_failed_start(std::string reason);
        static void shutdown_handler();
    public:
        /**
         * @brief 运行
         */
        static void Start();
        // 日志标签
        static const char * const LOG_TAG;
};

}

}

#endif // _application_hpp_
