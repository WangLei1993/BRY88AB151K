#ifndef _ze08_ch2o_hpp_
#define _ze08_ch2o_hpp_

#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace cubestone_wang 
{

namespace sensor
{

// ZE08_CH2O
class ZE08_CH2O
{
    public:
        // Data
        struct Data {
            uint16_t CH2O_UGM3;
            uint16_t CH2O_PPB;
        };
        // 日志标签
        static const char *const LOG_TAG;
        ZE08_CH2O(gpio_num_t rx, gpio_num_t tx, uart_port_t uart_num);
        ~ZE08_CH2O();
        Data GetData(); 
    private:
        SemaphoreHandle_t mutex;
        uart_port_t uart_num;
        void write(uint8_t* data, const uint32_t data_length);
        void read(uint8_t* data, const uint32_t data_length);
};

}

}

#endif // _ze08_ch2o_hpp_
