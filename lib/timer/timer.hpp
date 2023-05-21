#ifndef _timer_hpp_
#define _timer_hpp_

#include <string>
#include <map>
#include <set>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/queue.h"

namespace cubestone_wang 
{

namespace timer
{

class Timer
{
    public:
        // 日志标签
        static const char *const LOG_TAG;
        /**
         * @brief 回调函数定义
         */
        typedef void (* CallbackFunction_t)(void *args);
        /**
         * @brief 添加周期性事件
         *
         * @param name 事件名称
         * @param interval 重复执行的间隔(毫秒)
         * @param func 触发时的回调函数
         * @param args 回调函数参数
         */
        static bool AddPeriodicEvent(const std::string &name,
                                     const uint32_t interval,
                                     const CallbackFunction_t func, 
                                     void *args=nullptr);
        /**
         * @brief 添加一次性事件
         *
         * @param name 事件名称
         * @param delay 延迟的时间(毫秒)
         * @param func 触发时的回调函数
         * @param args 回调函数参数
         */
        static bool AddOneShotEvent(const std::string &name,
                                    const uint32_t delay,
                                     const CallbackFunction_t func, 
                                    void *args=nullptr);
        /**
         * @brief 删除
         *
         * @param name 事件名称
         */
        static bool Del(const std::string &name);
    private:
        struct Data{
            std::string Name;
            CallbackFunction_t Func; 
            void *Args;
            bool IsOneshot;
            esp_timer_handle_t Handle;
        };
        /**
         * @brief 初始化
         */
        static void init();
        static void deinit();
        static bool init_flag;
        static TaskHandle_t task_handler;
        static SemaphoreHandle_t mutex;
        static QueueHandle_t queue;
        static std::map<std::string, Data*> name_to_data;
        static std::set<Data*> exist_data;
        static void inner_timer_callback(void *args);
        static void run_task(void *args);
};

}

}

#endif // _mqtt_hpp_
