#ifndef _button_manager_hpp_
#define _button_manager_hpp_

#include <map>
#include <string>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

namespace cubestone_wang 
{

namespace button
{

using namespace std;

// 按钮管理类
class ButtonManager
{
    public:
        /**
         * @brief 回调函数定义
         */
        typedef void (* CallbackFunction_t)(void *);
    private:
        static void run_task(void *);
        static bool start_flag;
        static SemaphoreHandle_t mutex;
        enum class StateType {
            INIT,
            PRESS,
            PRESS_RELEASE,
            COUNT,
            LONG_PRESS,
            LONG_PRESS_RELEASE,
            UNKNOWN
        };
        // 按钮类
        class Button
        {
            public:
                static void run_task(void *_this);
                uint32_t debounce_ticks; 
                uint32_t click_ticks;  
                uint32_t press_ticks;  
                uint8_t button_pressed_level;  
                gpio_num_t pin;                            
                CallbackFunction_t click_callback_func;
                void *click_callback_func_args;
                CallbackFunction_t double_click_callback_func;
                void *double_click_callback_func_args;
                CallbackFunction_t multi_click_callback_func;
                void *multi_click_callback_func_args;
                CallbackFunction_t long_press_start_callback_func;
                void *long_press_start_callback_func_args;
                CallbackFunction_t long_press_stop_callback_func;
                void *long_press_stop_callback_func_args;
                CallbackFunction_t during_long_press_callback_func;
                void *during_long_press_callback_func_args;
                uint32_t start_time, wait_time;
                uint32_t click_count, max_click_count;
                bool button_pressed, has_long_press;
                StateType state, last_state;
        };
        static map<string, Button *> button_map;
        static TaskHandle_t task_handler;
    public:
        /**
         * @brief 添加
         * 
         * @param name 名称
         * @param button 实例
         */
        static bool Add(const string& name, const gpio_num_t pin, const uint8_t button_pressed_level=0);
        /**
         * @brief 删除
         * 
         * @param name 名称
         */
        static bool Del(const string& name);
        /**
         * @brief 启动
         * 
         * @param task_priority 任务优先级
         * @param stack_depth 任务栈深度
         */
        static bool Start(const UBaseType_t task_priority=2, const uint32_t stack_depth=4096);
        /**
         * @brief 停止
         */
        static void Stop();
        /**
         * @brief 设置单击的回调函数
         *
         * @param func 回调函数
         * @param args 回调函数参数
         */
        static bool SetClickCallbackFunction(const string& name, const CallbackFunction_t func=nullptr, void *args=nullptr);
        /**
         * @brief 设置双击的回调函数
         *
         * @param name 名称
         * @param func 回调函数
         * @param args 回调函数参数
         */
        static bool SetDoubleClickCallbackFunction(const string& name, const CallbackFunction_t func=nullptr, void *args=nullptr);
        /**
         * @brief 设置多击的回调函数
         *
         * @param name 名称
         * @param func 回调函数
         * @param args 回调函数参数
         */
        static bool SetMultiClickCallbackFunction(const string& name, const CallbackFunction_t func=nullptr, void *args=nullptr);
        /**
         * @brief 设置长按开始的回调函数
         *
         * @param name 名称
         * @param func 回调函数
         * @param args 回调函数参数
         */
        static bool SetLongPressStartCallbackFunction(const string& name, const CallbackFunction_t func=nullptr, void *args=nullptr);
        /**
         * @brief 设置长按停止的回调函数
         *
         * @param name 名称
         * @param func 回调函数
         * @param args 回调函数参数
         */
        static bool SetLongPressStopCallbackFunction(const string& name, const CallbackFunction_t func=nullptr, void *args=nullptr);
        /**
         * @brief 设置长按中的回调函数
         *
         * @param name 名称
         * @param func 回调函数
         * @param args 回调函数参数
         */
        static bool SetDuringLongPressCallbackFunction(const string& name, const CallbackFunction_t func=nullptr, void *args=nullptr);
        /**
         * @brief 设置消抖的判断周期
         *
         * @param name 名称
         * @param ticks 周期（毫秒）
         */
        static bool SetDebounceTicks(const string& name, const uint32_t ticks=50);
        /**
         * @brief 设置点击的判断周期
         *
         * @param name 名称
         * @param ticks 周期（毫秒）
         */
        static bool SetClickTicks(const string& name, const uint32_t ticks=400);
        /**
         * @brief 设置长按的判断周期
         *
         * @param name 名称
         * @param ticks 周期（毫秒）
         */
        static bool SetPressTicks(const string& name, const uint32_t ticks=800);
        // 日志标签
        static const char *const LOG_TAG;
    
};

}

}

#endif // _button_manager_hpp_
