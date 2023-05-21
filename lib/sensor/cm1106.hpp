#ifndef _cm1106_hpp_
#define _cm1106_hpp_

#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace cubestone_wang 
{

namespace sensor
{

// CM1106
class CM1106
{
    public:
        // 日志标签
        static const char *const LOG_TAG;
        CM1106(gpio_num_t rx, gpio_num_t tx, uart_port_t uart_num);
        ~CM1106();
        uint16_t GetPPM();
        void Calibrate(uint16_t ppm=400);
    private:
        SemaphoreHandle_t mutex;
        uart_port_t uart_num;
        void write(uint8_t* data, const uint32_t data_length);
        void read(uint8_t* data, const uint32_t data_length);
};

}

}

#endif // _cm1106_hpp_
