#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "system.hpp"
#include "timer.hpp"

#include "scheduled_restart.hpp"

namespace cubestone_wang 
{

namespace scheduled_restart
{

using namespace system;
using namespace timer;

const char *const ScheduledRestart::LOG_TAG = "SCHEDULED_RESTART";

bool ScheduledRestart::start_flag = false;

SemaphoreHandle_t ScheduledRestart::mutex = xSemaphoreCreateMutex();

const std::string ScheduledRestart::name = "scheduled_restart";

bool ScheduledRestart::Start(const DayType day_type, const uint8_t hour, const uint8_t minute)
{
    // 判断是否已经启动  
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (true == start_flag) {
        ESP_LOGI(LOG_TAG, "this has been started");
        // 退出临界区
        xSemaphoreGive(mutex);
        return false;
    }
    // 计算延迟时间
    uint32_t delay = 0;
    auto current_time_info = System::GetCurrentTime();
    if (DayType::RESTART_DAY_TYPE_EVERY_DAY == day_type) {
        // 每天执行
        if (hour < current_time_info.tm_hour || 
            (hour == current_time_info.tm_hour && minute < current_time_info.tm_min)) {
            delay += (24 - current_time_info.tm_hour + hour -1) * 60;
        } else {
            delay += (hour - current_time_info.tm_hour - 1) * 60;
        }
        delay += (60 - current_time_info.tm_min + minute);
    } else {
        // 特定星期几执行
        if (static_cast<int>(day_type) < current_time_info.tm_wday) {
            delay += (7 - current_time_info.tm_wday + static_cast<int>(day_type) - 1) * 24 * 60;
            delay += (24 - current_time_info.tm_hour + hour - 1) * 60;
            delay += (60 - current_time_info.tm_min + minute);
        } else if (static_cast<int>(day_type) == current_time_info.tm_wday) {
            if (hour < current_time_info.tm_hour || 
                (hour == current_time_info.tm_hour && minute < current_time_info.tm_min)) {
                delay += 6 * 24 * 60;
                delay += (24 - current_time_info.tm_hour + hour -1) * 60;
            } else {
                delay += (hour - current_time_info.tm_hour - 1) * 60;
            }
            delay += (60 - current_time_info.tm_min + minute);
        } else {
            delay += (static_cast<int>(day_type) - current_time_info.tm_wday - 1) * 24 * 60;
            delay += (24 - current_time_info.tm_hour - 1 + hour) * 60;
            delay += (60 - current_time_info.tm_min + minute);
        }
    }
    ESP_LOGD(LOG_TAG, "delay: %lu mintues", delay);
    delay = delay * 60 * 1000;
    if (Timer::AddOneShotEvent(name, 
                               delay,
                               [](void *args){
                                  System::Restart("scheduled restart", 3); 
                               },
                               nullptr)) {
        start_flag = true;
        // 退出临界区
        xSemaphoreGive(mutex);
        return true;
    } else {
        start_flag = false;
        // 退出临界区
        xSemaphoreGive(mutex);
        return false;
    }
}

void ScheduledRestart::Stop()
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (false == start_flag) { 
        ESP_LOGD(LOG_TAG, "this has been stopped");
        goto DONE;
    }
    Timer::Del(name);
DONE:
    start_flag = false;
    // 退出临界区
    xSemaphoreGive(mutex);
    return;
}

}

}