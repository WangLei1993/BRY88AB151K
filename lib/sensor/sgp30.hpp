#ifndef _sgp30_hpp_
#define _sgp30_hpp_

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "i2c_master.hpp"

namespace cubestone_wang 
{

namespace sensor 
{

using namespace i2c_master;

// SGP30
class SGP30
{
    public:
        // Data
        struct Data {
            uint16_t CO2eq;
            uint16_t TVOC;
        };
        // Baseline
        struct Baseline {
            uint16_t CO2eq;
            uint16_t TVOC;
        };
        // 日志标签
        static const char *const LOG_TAG;
        SGP30(I2cMaster* i2c_master);
        /**
         * @brief 获取数据
         */
        Data GetData(float temperature, float humidity);
        Baseline GetBaseline();
        void SetBaseline(Baseline baseline);
    private:
        SemaphoreHandle_t mutex;
        I2cMaster* i2c_master;
        static uint8_t device_address;
        uint8_t crc(uint8_t data1, uint8_t data2);
};

}

}

#endif // _sgp30_hpp_
