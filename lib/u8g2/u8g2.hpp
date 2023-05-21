#ifndef _u8g2_hpp_
#define _u8g2_hpp_

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "u8g2.h"

#include "i2c_master.hpp"

namespace cubestone_wang 
{

namespace u8g2
{

using namespace i2c_master;

class U8G2
{
    public:
        // 日志标签
        static const char *const LOG_TAG;
        enum class DeviceType {
            SSD1306_I2C_128x64,
            SH1106_I2C_128x64,
            UNKNOWN
        };
        U8G2(I2cMaster* const i2c_master, 
             const uint8_t i2c_device_address, 
             const DeviceType device_type, 
             const u8g2_cb_t* rotation);
        virtual ~U8G2();
        u8g2_t* GetInstance();
        void Lock();
        void Unlock();
        static uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, 
                                           uint8_t msg, 
                                           uint8_t arg_int, 
                                           void *arg_ptr);
        static uint8_t u8x8_byte_i2c(u8x8_t *u8x8, 
                                     uint8_t msg, 
                                     uint8_t arg_int, 
                                     void *arg_ptr);
    private:
        SemaphoreHandle_t mutex;
        u8g2_t instance;
        I2cMaster* i2c_master;
        uint8_t i2c_device_address;
        uint8_t buffer[32];
        uint8_t buffer_length;
};

}

}

#endif // _u8g2_hpp_
