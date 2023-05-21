#ifndef _event_loop_hpp_
#define _event_loop_hpp_

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace cubestone_wang 
{

namespace event_loop 
{

// 事件循环类
class EventLoop
{
    public:
        /**
         * @brief 初始化
         */
        static void Init();
        // 日志标签
        static const char *const LOG_TAG;
    private:
        static bool init_flag;
        static SemaphoreHandle_t mutex;
};

}

}

#endif // _event_loop_hpp_
