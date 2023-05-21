#ifndef _monochrome_led_manager_hpp_
#define _monochrome_led_manager_hpp_

#include <map>
#include <string>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

namespace cubestone_wang 
{

namespace monochrome_led
{

using namespace std;

// 单色LED管理类
class MonochromeLEDManager
{
    public:
        /**
         * @brief 添加
         * 
         * @param name 名称
         * @param gpio_num_t GPIO
         * @param reverse_on_off 是否反转亮灭
         */
        static void Add(const string& name, const gpio_num_t pin, const bool reverse_on_off=false);
        /**
         * @brief 删除
         * 
         * @param name 名称
         */
        static void Del(const string& name);
        /**
         * @brief 启动
         * 
         * @param task_priority 任务优先级
         * @param stack_depth 任务栈深度
         */
        static bool Start(const UBaseType_t task_priority=1, const uint32_t stack_depth=3072);
        /**
         * @brief 停止
         */
        static void Stop();
        /**
         * @brief 点亮LED
         * 
         * @param name 名称
         */
        static void SetOn(const string& name);
        /**
         * @brief 熄灭LED
         * 
         * @param name 名称
         */
        static void SetOff(const string& name);
        /**
         * @brief 闪烁LED
         * 
         * @param name 名称
         * @param on 亮（毫秒）
         * @param off 灭（毫秒）
         */
        static void SetBlink(const string& name, const uint32_t on=100, const uint32_t off=100);
        /**
         * @brief 闪烁LED
         * 
         * @param name 名称
         * @param on 亮（毫秒）
         * @param off 灭（毫秒）
         */
        // 日志标签
        static const char *const LOG_TAG;
    private:
        // 单色LED类
        class MonochromeLED
        {   
            public:
                uint32_t GPIO_LEVEL_ON, GPIO_LEVEL_OFF;
                gpio_num_t pin;                            
                uint32_t on, off;   
                uint32_t last_gpio_level;
                uint32_t last_on;
                uint32_t last_off;
                uint32_t last_change_timestamp;      
        };
        static void run_task(void *);
        static bool start_flag;
        static SemaphoreHandle_t mutex;
        static map<string, MonochromeLED *> monochrome_led_map;
        static TaskHandle_t task_handler;
};

}

}

#endif // _monochrome_led_manager_hpp_
