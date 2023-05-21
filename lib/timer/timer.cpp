#include "esp_log.h"

#include "system.hpp"

#include "timer.hpp"

namespace cubestone_wang 
{

namespace timer
{

const char *const Timer::LOG_TAG = "TIMER";

bool Timer::init_flag = false;
std::map<std::string, Timer::Data*> Timer::name_to_data;
std::set<Timer::Data*> Timer::exist_data;
SemaphoreHandle_t Timer::mutex = xSemaphoreCreateRecursiveMutex();
TaskHandle_t Timer::task_handler = nullptr;
QueueHandle_t Timer::queue = xQueueCreate(20, sizeof(Timer::Data *));

void Timer::init() 
{
    // 判断是否已经初始化  
    if (true == init_flag) {
        ESP_LOGI(LOG_TAG, "this has been inited");
        return;
    }
    auto err = xTaskCreate(run_task, 
                           "timer", 
                           4096, 
                           nullptr, 
                           15, 
                           &task_handler);
    if (err != pdPASS) {
        ESP_LOGE(LOG_TAG, "init err, the reason is %d", err);
    }
    init_flag = true;
    return;
}

void Timer::deinit()
{
    // 判断是否未初始化  
    if (false == init_flag) {
        ESP_LOGI(LOG_TAG, "this has not been inited");
        return;
    }
    vTaskDelete(task_handler);
    task_handler = nullptr;
    xQueueReset(queue);
    init_flag = false;
    return;
}

void Timer::run_task(void *args)
{   
    while(1) {
        Data *data;
        if (pdTRUE != xQueueReceive(queue, (void *)&data, portMAX_DELAY)) {
            continue;
        }
        // 设置临界区
        xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
        if (0 == exist_data.count(data)) {
            // 退出临界区
            xSemaphoreGiveRecursive(mutex);
            continue;
        }
        bool is_oneshot = data->IsOneshot;
        auto func = data->Func;
        auto args = data->Args;
        if (true == is_oneshot) {
            esp_timer_stop(data->Handle);
            ESP_ERROR_CHECK(esp_timer_delete(data->Handle));
            exist_data.erase(data);
            name_to_data.erase(data->Name);
            delete data;
        }
        if (func) {
            try {
                func(args);
            } catch(const std::exception& e) {
                ESP_LOGE(LOG_TAG, "unexpected->%s", e.what());
            }
        }
        if (true == is_oneshot) {
            if (name_to_data.empty()) {
                task_handler = nullptr;
                xQueueReset(queue);
                init_flag = false;
                // 退出临界区
                xSemaphoreGiveRecursive(mutex);
                vTaskDelete(NULL);
            }
        }
        // 退出临界区
        xSemaphoreGiveRecursive(mutex);
    }
}

void Timer::inner_timer_callback(void *args) 
{
    xQueueSend(queue, (void *)&args, portMAX_DELAY);
}

bool Timer::AddPeriodicEvent(const std::string &name,
                             const uint32_t interval,
                             const CallbackFunction_t func, 
                             void *args)
{
    // 设置临界区
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    if (name_to_data.count(name) > 0) {
        ESP_LOGE(LOG_TAG, "%s has been exist", name.c_str());
        // 退出临界区
        xSemaphoreGiveRecursive(mutex);
        return false;
    }
    // 判断是否已经初始化  
    if (false == init_flag) {
        init();
    }
    Data *data = new Data();
    esp_timer_create_args_t timer_create_args;
    esp_timer_handle_t timer_handle = nullptr;
    timer_create_args.dispatch_method = ESP_TIMER_TASK;
    timer_create_args.callback = inner_timer_callback;
    timer_create_args.arg = (void *)data;
    ESP_ERROR_CHECK(esp_timer_create(&timer_create_args, &timer_handle));
    data->Func = func;
    data->Args = args;
    data->IsOneshot = false;
    data->Handle = timer_handle;
    data->Name = name;
    name_to_data[name] = data;
    exist_data.insert(data);
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle, interval*(uint64_t)1000));
    // 退出临界区
    xSemaphoreGiveRecursive(mutex);
    return true;
}

bool Timer::AddOneShotEvent(const std::string &name,
                            const uint32_t delay,
                            const CallbackFunction_t func, 
                            void *args)
{
    // 设置临界区
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    if (name_to_data.count(name) > 0) {
        ESP_LOGE(LOG_TAG, "%s has been exist", name.c_str());
        // 退出临界区
        xSemaphoreGiveRecursive(mutex);
        return false;
    }
    // 判断是否已经初始化  
    if (false == init_flag) {
        init();  
    }
    Data *data = new Data();
    esp_timer_create_args_t timer_create_args;
    esp_timer_handle_t timer_handle = nullptr;
    timer_create_args.dispatch_method = ESP_TIMER_TASK;
    timer_create_args.callback = inner_timer_callback;
    timer_create_args.arg = (void *)data;
    ESP_ERROR_CHECK(esp_timer_create(&timer_create_args, &timer_handle));
    data->Func = func;
    data->Args = args;
    data->IsOneshot = true;
    data->Handle = timer_handle;
    data->Name = name;
    name_to_data[name] = data;
    exist_data.insert(data);
    ESP_ERROR_CHECK(esp_timer_start_once(timer_handle, delay*(uint64_t)1000));
    // 退出临界区
    xSemaphoreGiveRecursive(mutex);
    return true;
}

bool Timer::Del(const std::string &name)
{
    // 设置临界区
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    auto iter = name_to_data.find(name);  
    if(iter == name_to_data.end()) {
        ESP_LOGW(LOG_TAG, "%s can't be found", name.c_str());
        // 退出临界区
        xSemaphoreGiveRecursive(mutex);
        return false;
    }
    if (0 < exist_data.count(iter->second)) {
        exist_data.erase(iter->second);
    }
    esp_timer_stop(iter->second->Handle);
    ESP_ERROR_CHECK(esp_timer_delete(iter->second->Handle));
    delete iter->second;
    name_to_data.erase(iter);
    if (name_to_data.empty()) {
        deinit();
    }
    // 退出临界区
    xSemaphoreGiveRecursive(mutex);
    return true;
}

}

}
