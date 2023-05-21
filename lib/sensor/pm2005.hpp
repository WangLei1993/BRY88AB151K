#ifndef _pm2005_hpp_
#define _pm2005_hpp_

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "i2c_master.hpp"

namespace cubestone_wang 
{

namespace sensor 
{

using namespace i2c_master;

// PM2005
class PM2005
{
    public:
        // Data
        struct Data {
            uint16_t PM25;
            uint16_t PM10;
        };
        // 日志标签
        static const char *const LOG_TAG;
        PM2005(I2cMaster* i2c_master);
        /**
         * @brief 获取数据
         */
        Data GetData();
    private:
        SemaphoreHandle_t mutex;
        I2cMaster* i2c_master;
        static uint8_t device_address;
};

}

}

#endif // _pm2005_hpp_
