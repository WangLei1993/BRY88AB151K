#ifndef _hdc1080_hpp_
#define _hdc1080_hpp_

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "i2c_master.hpp"

namespace cubestone_wang 
{

namespace sensor 
{

using namespace i2c_master;

// HDC1080
class HDC1080
{
    public:
        // 日志标签
        static const char *const LOG_TAG;
        HDC1080(I2cMaster* i2c_master);
        /**
         * @brief 获取温度
         */
        float GetTemperature(float offset=0);
        /**
         * @brief 获取湿度
         */
        float GetHumidity(float offset=0);
    private:
        SemaphoreHandle_t mutex;
        I2cMaster* i2c_master;
        static uint8_t device_address;
};

}

}

#endif // _hdc1080_hpp_
