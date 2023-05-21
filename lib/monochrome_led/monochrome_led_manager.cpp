#include <map>
#include <string>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "system.hpp"

#include "monochrome_led_manager.hpp"

namespace cubestone_wang 
{

namespace monochrome_led
{

using namespace std;
using namespace system;

const char *const MonochromeLEDManager::LOG_TAG = "MONOCHROME_LED_MANAGER";

bool MonochromeLEDManager::start_flag = false;

SemaphoreHandle_t MonochromeLEDManager::mutex = xSemaphoreCreateMutex();

map<string, MonochromeLEDManager::MonochromeLED *> MonochromeLEDManager::monochrome_led_map = map<string, MonochromeLEDManager::MonochromeLED *>();

TaskHandle_t MonochromeLEDManager::task_handler = nullptr;

void MonochromeLEDManager::Add(const string& name, const gpio_num_t pin, const bool reverse_on_off)
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (monochrome_led_map.count(name) > 0) {
        ESP_LOGE(LOG_TAG, "%s monochrome LED have been exist", name.c_str());
        // 退出临界区
        xSemaphoreGive(mutex);
        return;
    }
    auto monochrome_led = new MonochromeLED();
    monochrome_led->pin = pin;
    if (reverse_on_off) {
        monochrome_led->GPIO_LEVEL_ON = 0;
        monochrome_led->GPIO_LEVEL_OFF = 1;
    } else {
        monochrome_led->GPIO_LEVEL_ON = 1;
        monochrome_led->GPIO_LEVEL_OFF = 0;
    }
    ESP_ERROR_CHECK(gpio_reset_pin(monochrome_led->pin));
    ESP_ERROR_CHECK(gpio_set_intr_type(monochrome_led->pin, GPIO_INTR_DISABLE));
    ESP_ERROR_CHECK(gpio_set_direction(monochrome_led->pin, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_level(monochrome_led->pin, monochrome_led->GPIO_LEVEL_OFF));
    monochrome_led->on = 0;
    monochrome_led->off = 100;
    monochrome_led->last_gpio_level = monochrome_led->GPIO_LEVEL_OFF;
    monochrome_led->last_on = 0;
    monochrome_led->last_off = 0;
    monochrome_led->last_change_timestamp = 0;
    monochrome_led_map[name] = monochrome_led;
    ESP_LOGD(LOG_TAG, "%s add", name.c_str());
    // 退出临界区
    xSemaphoreGive(mutex);
}

void MonochromeLEDManager::Del(const string& name)
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (monochrome_led_map.count(name) == 0) {
        ESP_LOGE(LOG_TAG, "%s can't be found", name.c_str());
        // 退出临界区
        xSemaphoreGive(mutex);
        return;
    }
    monochrome_led_map.erase(name);
    ESP_LOGD(LOG_TAG, "%s del", name.c_str());
    // 退出临界区
    xSemaphoreGive(mutex);
}

bool MonochromeLEDManager::Start(const UBaseType_t task_priority, const uint32_t stack_depth)
{  
    // 判断是否已经启动  
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (start_flag == true) {
        ESP_LOGI(LOG_TAG, "this has been started");
        // 退出临界区
        xSemaphoreGive(mutex);
        return false;
    }
    start_flag = true;
    auto err = xTaskCreate(run_task, 
                           "monochrome_led", 
                           stack_depth, 
                           nullptr, 
                           task_priority, 
                           &task_handler);
    // 退出临界区
    xSemaphoreGive(mutex);
    return err == pdPASS;
}

void MonochromeLEDManager::SetOn(const string& name)
{
    MonochromeLEDManager::SetBlink(name, 100, 0);
}

void MonochromeLEDManager::SetOff(const string& name)
{
    MonochromeLEDManager::SetBlink(name, 0, 100);
}

void MonochromeLEDManager::SetBlink(const string& name, const uint32_t on, const uint32_t off)
{   
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    auto iter = monochrome_led_map.find(name);  
    if(iter == monochrome_led_map.end()) {
        ESP_LOGE(LOG_TAG, "%s can't be found", name.c_str());
        // 退出临界区
        xSemaphoreGive(mutex);
        return;
    }
    auto monochrome_led = iter->second;
    if (on == 0 && off == 0)
    {
        ESP_LOGE(LOG_TAG, "%s off %% 50 must equal to 0, set error", name.c_str());
        // 退出临界区
        xSemaphoreGive(mutex);
        return;
    }
    if (on % 50 != 0)
    {
        ESP_LOGE(LOG_TAG, "%s on %% 50 must equal to 0, set error", name.c_str());
        // 退出临界区
        xSemaphoreGive(mutex);
        return;
    }
    if (off % 50 != 0)
    {
        ESP_LOGE(LOG_TAG, "%s off %% 50 must equal to 0, set error", name.c_str());
        // 退出临界区
        xSemaphoreGive(mutex);
        return;
    }
    monochrome_led->on = on;
    monochrome_led->off = off;
    if (off == 0) {
        ESP_LOGD(LOG_TAG, "%s always on", name.c_str());
    } else if (on == 0) {
        ESP_LOGD(LOG_TAG, "%s always off", name.c_str());
    } else {
        ESP_LOGD(LOG_TAG, "%s blink, on %lu ms, off %lu ms", 
                 name.c_str(),
                 monochrome_led->on,
                 monochrome_led->off);
    }
    // 退出临界区
    xSemaphoreGive(mutex);
}

void MonochromeLEDManager::run_task(void *) 
{
    ESP_LOGD(LOG_TAG, "running");
    uint32_t current_timestamp = esp_log_timestamp();;
    uint32_t timestamp_interval;
    for (auto iter = monochrome_led_map.begin(); iter != monochrome_led_map.end(); iter++) 
    {
        iter->second->last_change_timestamp = current_timestamp;
    }
    while (true) 
    {   
        // 设置临界区
        xSemaphoreTake(mutex, portMAX_DELAY);
        // 处理每一个灯
        for (auto iter = monochrome_led_map.begin(); iter != monochrome_led_map.end(); iter++) 
        {
            auto monochrome_led = iter->second;
            if (monochrome_led->last_on == monochrome_led->on 
                && monochrome_led->last_off == monochrome_led->off
                && monochrome_led->on != 0
                && monochrome_led->off != 0) {
                // 还是之前的设置, 则计算等待时间进行处理，只需要处理闪烁的情况
                current_timestamp = esp_log_timestamp();
                if (current_timestamp >= monochrome_led->last_change_timestamp) {
                    timestamp_interval = current_timestamp - monochrome_led->last_change_timestamp;
                } else {
                    // 处理时间戳溢出问题
                    timestamp_interval = UINT32_MAX - monochrome_led->last_change_timestamp + current_timestamp;
                }
                if (monochrome_led->last_gpio_level == monochrome_led->GPIO_LEVEL_ON) {
                    // 之前是亮的，看是否需要切换到灭
                    if (timestamp_interval >= monochrome_led->on) {
                        ESP_ERROR_CHECK(gpio_set_level(monochrome_led->pin, monochrome_led->GPIO_LEVEL_OFF));
                        monochrome_led->last_gpio_level = monochrome_led->GPIO_LEVEL_OFF;
                        monochrome_led->last_change_timestamp = esp_log_timestamp();
                    }
                } else {
                    // 之前是灭的，看是否需要切换到亮
                    if (timestamp_interval >= monochrome_led->off) {
                        ESP_ERROR_CHECK(gpio_set_level(monochrome_led->pin, monochrome_led->GPIO_LEVEL_ON));
                        monochrome_led->last_gpio_level = monochrome_led->GPIO_LEVEL_ON;
                        monochrome_led->last_change_timestamp = esp_log_timestamp();
                    }
                }
            } else {
                // 变成新的设置了
                if (monochrome_led->off == 0) {
                    // 常亮
                    if (monochrome_led->last_gpio_level == monochrome_led->GPIO_LEVEL_OFF) {
                        ESP_ERROR_CHECK(gpio_set_level(monochrome_led->pin, monochrome_led->GPIO_LEVEL_ON));
                        monochrome_led->last_gpio_level = monochrome_led->GPIO_LEVEL_ON;
                    }
                } else if (monochrome_led->on == 0) {
                    // 常灭
                    if (monochrome_led->last_gpio_level == monochrome_led->GPIO_LEVEL_ON) {
                        ESP_ERROR_CHECK(gpio_set_level(monochrome_led->pin, monochrome_led->GPIO_LEVEL_OFF));
                        monochrome_led->last_gpio_level = monochrome_led->GPIO_LEVEL_OFF;
                    }
                } else {
                    // 闪烁
                    ESP_ERROR_CHECK(gpio_set_level(monochrome_led->pin, monochrome_led->GPIO_LEVEL_ON));
                    monochrome_led->last_gpio_level = monochrome_led->GPIO_LEVEL_ON;
                    monochrome_led->last_change_timestamp = esp_log_timestamp();
                }
                monochrome_led->last_on = monochrome_led->on;
                monochrome_led->last_off = monochrome_led->off;
            }
        }
        // 退出临界区
        xSemaphoreGive(mutex);
        //休眠至下一处理周期
        System::Sleep(10);
    }
}

void MonochromeLEDManager::Stop()
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (start_flag == false) { 
        ESP_LOGD(LOG_TAG, "this has been stopped");
        goto DONE;
    }
    vTaskDelete(task_handler);
    task_handler = nullptr;
DONE:
    start_flag = false;
    // 退出临界区
    xSemaphoreGive(mutex);
    return;
}

}

}
