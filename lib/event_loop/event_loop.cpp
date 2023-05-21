#include "string.h"

#include "esp_log.h"
#include "esp_event.h"

#include "event_loop.hpp"

namespace cubestone_wang 
{

namespace event_loop 
{

const char *const EventLoop::LOG_TAG = "EVENT_LOOP";

SemaphoreHandle_t EventLoop::mutex = xSemaphoreCreateMutex();

bool EventLoop::init_flag = false;

void EventLoop::Init() 
{
    // 判断是否已经执行过 
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (init_flag == true) {
        ESP_LOGD(LOG_TAG, " it have been inited");
        // 退出临界区
        xSemaphoreGive(mutex);
        return;
    }
    init_flag = true;
    // 初始化数据
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // 退出临界区
    xSemaphoreGive(mutex);
    return;
}

}

}
