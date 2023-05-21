#ifndef _i2c_master_hpp_
#define _i2c_master_hpp_

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace cubestone_wang 
{

namespace i2c_master
{

// I2cMaster
class I2cMaster
{
    public:
        // 日志标签
        static const char *const LOG_TAG;
        I2cMaster(gpio_num_t sda, 
                  gpio_num_t scl, 
                  uint32_t clk_speed, 
                  i2c_port_t i2c_num);
        ~I2cMaster();
        void Write(const uint8_t device_address, 
                   const uint8_t* const write_buffer, 
                   const size_t write_size);
        void Read(const uint8_t device_address, 
                  uint8_t* read_buffer, 
                  const size_t read_size);
        void ReadAfterWrite(const uint8_t device_address, 
                            const uint8_t* const write_buffer, 
                            const size_t write_size, 
                            uint8_t* read_buffer, 
                            const size_t read_size);
        void SearchAddress();
    private:
        SemaphoreHandle_t mutex;
        i2c_port_t i2c_num;
};

}

}

#endif // _i2c_master_hpp_
