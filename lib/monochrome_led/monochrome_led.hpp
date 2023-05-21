#ifndef _monochrome_led_hpp_
#define _monochrome_led_hpp_

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace cubestone_wang 
{

namespace monochrome_led
{

// 单色LED类
class MonochromeLED
{   
    private:
        uint32_t GPIO_LEVEL_ON, GPIO_LEVEL_OFF;
        gpio_num_t pin;  
        SemaphoreHandle_t mutex;
    public:
        MonochromeLED(const gpio_num_t pin, const bool reverse_on_off=false);
        void SetOn();
        void SetOff();
        // 日志标签
        static const char *const LOG_TAG;
};

}

}

#endif // _monochrome_led_hpp_
