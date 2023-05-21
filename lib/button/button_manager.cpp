#include <map>
#include <string>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "system.hpp"

#include "button_manager.hpp"

namespace cubestone_wang 
{

namespace button
{

using namespace std;
using namespace system;

const char *const ButtonManager::LOG_TAG = "BUTTON_MANAGER";

bool ButtonManager::start_flag = false;

SemaphoreHandle_t ButtonManager::mutex = xSemaphoreCreateMutex();

map<string, ButtonManager::Button *> ButtonManager::button_map = map<string, ButtonManager::Button *>();

TaskHandle_t ButtonManager::task_handler = nullptr;

bool ButtonManager::Add(const string& name, const gpio_num_t pin, const uint8_t button_pressed_level)
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (button_map.count(name) > 0) {
        ESP_LOGE(LOG_TAG, "%s has been exist", name.c_str());
        // 退出临界区
        xSemaphoreGive(mutex);
        return false;
    }
    auto button = new Button();
    button->pin = pin;
    button->button_pressed_level = button_pressed_level;
    ESP_ERROR_CHECK(gpio_reset_pin(button->pin));
    ESP_ERROR_CHECK(gpio_set_intr_type(button->pin, GPIO_INTR_DISABLE));
    ESP_ERROR_CHECK(gpio_set_direction(button->pin, GPIO_MODE_INPUT));
    if (button->button_pressed_level) {
        ESP_ERROR_CHECK(gpio_set_pull_mode(button->pin, GPIO_PULLDOWN_ONLY));
    } else {
        ESP_ERROR_CHECK(gpio_set_pull_mode(button->pin, GPIO_PULLUP_ONLY));
    }
    button->debounce_ticks = 50;
    button->click_ticks = 400;
    button->press_ticks = 800;
    button->click_callback_func = nullptr;
    button->click_callback_func_args = nullptr;
    button->double_click_callback_func = nullptr;
    button->double_click_callback_func_args = nullptr;
    button->multi_click_callback_func = nullptr;
    button->multi_click_callback_func_args = nullptr;
    button->long_press_start_callback_func = nullptr;
    button->long_press_start_callback_func_args = nullptr;
    button->long_press_stop_callback_func = nullptr;
    button->long_press_stop_callback_func_args = nullptr;
    button->during_long_press_callback_func = nullptr;
    button->during_long_press_callback_func_args = nullptr;
    button->state = button->last_state = StateType::INIT;
    button->click_count = 0;
    button->start_time = 0;
    button_map[name] = button;
    ESP_LOGD(LOG_TAG, "%s add", name.c_str());
    // 退出临界区
    xSemaphoreGive(mutex);
    return true;
}

bool ButtonManager::Del(const string& name)
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (button_map.count(name) == 0) {
        ESP_LOGW(LOG_TAG, "%s can't be found", name.c_str());
        // 退出临界区
        xSemaphoreGive(mutex);
        return false;
    }
    button_map.erase(name);
    ESP_LOGD(LOG_TAG, "%s del", name.c_str());
    // 退出临界区
    xSemaphoreGive(mutex);
    return true;
}

bool ButtonManager::Start(const UBaseType_t task_priority, const uint32_t stack_depth)
{  
    // 判断是否已经启动  
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    // ESP_LOGD(LOG_TAG, "mutex return code: %d", err);
    if (start_flag == true) {
        ESP_LOGI(LOG_TAG, "this has been started");
        // 退出临界区
        xSemaphoreGive(mutex);
        return false;
    }
    ButtonManager::start_flag = true;
    auto err = xTaskCreate(run_task, 
                           "button", 
                           stack_depth, 
                           nullptr, 
                           task_priority, 
                           &task_handler);
    // 退出临界区
    xSemaphoreGive(mutex);
    return err == pdPASS;
}

bool ButtonManager::SetClickCallbackFunction(const string& name, const CallbackFunction_t func, void *args)
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    auto iter = button_map.find(name);  
    if(iter == button_map.end()) {
        ESP_LOGE(LOG_TAG, "%s can't be found", name.c_str());
        // 退出临界区
        xSemaphoreGive(mutex);
        return false;
    }
    auto button = iter->second;
    button->click_callback_func = func;
    button->click_callback_func_args = args;
    // 退出临界区
    xSemaphoreGive(mutex);
    return true;
}

bool ButtonManager::SetDoubleClickCallbackFunction(const string& name, const CallbackFunction_t func, void *args)
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    auto iter = button_map.find(name);  
    if(iter == button_map.end()) {
        ESP_LOGE(LOG_TAG, "%s can't be found", name.c_str());
        // 退出临界区
        xSemaphoreGive(mutex);
        return false;
    }
    auto button = iter->second;
    button->double_click_callback_func = func;
    button->double_click_callback_func_args = args;
    // 退出临界区
    xSemaphoreGive(mutex);
    return true;
}

bool ButtonManager::SetMultiClickCallbackFunction(const string& name, const CallbackFunction_t func, void *args)
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    auto iter = button_map.find(name);  
    if(iter == button_map.end()) {
        ESP_LOGE(LOG_TAG, "%s can't be found", name.c_str());
        // 退出临界区
        xSemaphoreGive(mutex);
        return false;
    }
    auto button = iter->second;
    button->multi_click_callback_func = func;
    button->multi_click_callback_func_args = args;
    // 退出临界区
    xSemaphoreGive(mutex);
    return true;
}

bool ButtonManager::SetLongPressStartCallbackFunction(const string& name, const CallbackFunction_t func, void *args)
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    auto iter = button_map.find(name);  
    if(iter == button_map.end()) {
        ESP_LOGE(LOG_TAG, "%s can't be found", name.c_str());
        // 退出临界区
        xSemaphoreGive(mutex);
        return false;
    }
    auto button = iter->second;
    button->long_press_start_callback_func = func;
    button->long_press_start_callback_func_args = args;
    // 退出临界区
    xSemaphoreGive(mutex);
    return true;
}

bool ButtonManager::SetLongPressStopCallbackFunction(const string& name, const CallbackFunction_t func, void *args)
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    auto iter = button_map.find(name);  
    if(iter == button_map.end()) {
        ESP_LOGE(LOG_TAG, "%s can't be found", name.c_str());
        // 退出临界区
        xSemaphoreGive(mutex);
        return false;
    }
    auto button = iter->second;
    button->long_press_stop_callback_func = func;
    button->long_press_stop_callback_func_args = args;
    // 退出临界区
    xSemaphoreGive(mutex);
    return true;
}

bool ButtonManager::SetDuringLongPressCallbackFunction(const string& name, const CallbackFunction_t func, void *args)
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    auto iter = button_map.find(name);  
    if(iter == button_map.end()) {
        ESP_LOGE(LOG_TAG, "%s can't be found", name.c_str());
        // 退出临界区
        xSemaphoreGive(mutex);
        return false;
    }
    auto button = iter->second;
    button->during_long_press_callback_func = func;
    button->during_long_press_callback_func_args = args;
    // 退出临界区
    xSemaphoreGive(mutex);
    return true;
}

bool ButtonManager::SetDebounceTicks(const string& name, const uint32_t ticks)
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    auto iter = button_map.find(name);  
    if(iter == button_map.end()) {
        ESP_LOGE(LOG_TAG, "%s can't be found", name.c_str());
        // 退出临界区
        xSemaphoreGive(mutex);
        return false;
    }
    auto button = iter->second;
    button->debounce_ticks = ticks;
    // 退出临界区
    xSemaphoreGive(mutex);
    return true;
}

bool ButtonManager::SetClickTicks(const string& name, const uint32_t ticks)
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    auto iter = button_map.find(name);  
    if(iter == button_map.end()) {
        ESP_LOGE(LOG_TAG, "%s can't be found", name.c_str());
        // 退出临界区
        xSemaphoreGive(mutex);
        return false;
    }
    auto button = iter->second;
    button->click_ticks = ticks;
    // 退出临界区
    xSemaphoreGive(mutex);
    return true;
}

bool ButtonManager::SetPressTicks(const string& name, const uint32_t ticks)
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    auto iter = button_map.find(name);  
    if(iter == button_map.end()) {
        ESP_LOGE(LOG_TAG, "%s can't be found", name.c_str());
        // 退出临界区
        xSemaphoreGive(mutex);
        return false;
    }
    auto button = iter->second;
    button->press_ticks = ticks;
    // 退出临界区
    xSemaphoreGive(mutex);
    return true;
}

void ButtonManager::run_task(void *) {
    ESP_LOGD(LOG_TAG, "running");
    auto tmp_map = [](StateType _state)
    {
        switch (_state)
        {
        case StateType::INIT:
            return "INIT";
        case StateType::PRESS:
            return "PRESS";
        case StateType::PRESS_RELEASE:
            return "PRESS_RELEASE";
        case StateType::LONG_PRESS:
            return "LONG_PRESS";
        case StateType::LONG_PRESS_RELEASE:
            return "LONG_PRESS_RELEASE";
        case StateType::COUNT:
            return "COUNT";
        default:
            return "UNKWON";
        }
    };
    auto new_state = [&tmp_map](string& button_name, Button *button, StateType state)
    { 
        ESP_LOGV(LOG_TAG, "%s %s to %s", button_name.c_str(), tmp_map(button->last_state), tmp_map(button->state));
        button->last_state = state; 
        button->state = state;
    };
    auto reset = [](Button *button)
    {
        button->state = button->last_state = StateType::INIT;
        button->click_count = 0;
        button->start_time = 0;
    };
    for (auto iter = button_map.begin(); iter != button_map.end(); iter++) 
    {
        reset(iter->second);
    }
    while(true)
    {   
        // 设置临界区
        xSemaphoreTake(mutex, portMAX_DELAY);
        // 处理每一个按钮
        for (auto iter = button_map.begin(); iter != button_map.end(); iter++) 
        {
            auto button_name = iter->first;
            auto button = iter->second;
            // 设置最大点击数
            if (button->multi_click_callback_func != nullptr) {
                button->max_click_count = 100;
            } else if (button->double_click_callback_func != nullptr) {
                button->max_click_count = 2;
            } else {
                button->max_click_count = 1;
            }
            // 判断是否开启长按判断
            if (button->long_press_start_callback_func == nullptr && 
                button->during_long_press_callback_func == nullptr &&
                button->long_press_stop_callback_func == nullptr) {
                button->has_long_press = false;
            } else {
                button->has_long_press = true;
            }
            // 判断按钮状态
            button->button_pressed = gpio_get_level(button->pin) == button->button_pressed_level ? true : false; 
            // 处理时间
            auto current_time = esp_log_timestamp();
            if (current_time >= button->start_time) {
                button->wait_time = current_time - button->start_time;
            } else {
                // 处理时间戳溢出问题
                button->wait_time = UINT32_MAX - button->start_time + current_time;
            }
            switch (button->state)
            {
                case StateType::INIT:
                    if (button->button_pressed) {
                        new_state(button_name, button, StateType::PRESS);
                        button->start_time = esp_log_timestamp();
                        button->click_count = 0;
                    }
                    break;
                case StateType::PRESS:
                    if (button->button_pressed) { 
                        if (button->has_long_press) {
                            if (button->wait_time > button->press_ticks) {
                                ESP_LOGD(LOG_TAG, "%s long press start", button_name.c_str());
                                if (button->long_press_start_callback_func != nullptr) {
                                    try {
                                        button->long_press_start_callback_func(button->long_press_start_callback_func_args);
                                    } catch(const std::exception& e) {
                                        ESP_LOGE(LOG_TAG, "unexpected->%s", e.what());
                                    }
                                }
                                ESP_LOGD(LOG_TAG, "%s during long press", button_name.c_str());
                                if (button->during_long_press_callback_func != nullptr) {
                                    try {
                                        button->during_long_press_callback_func(button->during_long_press_callback_func_args);
                                    } catch(const std::exception& e) {
                                        ESP_LOGE(LOG_TAG, "unexpected->%s", e.what());
                                    }
                                }
                                new_state(button_name, button, StateType::LONG_PRESS);
                            }
                        }
                    } else {
                        if (button->wait_time < button->debounce_ticks) {
                            new_state(button_name, button, button->last_state);
                        } else {
                            new_state(button_name, button, StateType::PRESS_RELEASE);
                            button->start_time = esp_log_timestamp();
                        }
                    }
                    break;
                case StateType::PRESS_RELEASE:
                    if (button->button_pressed) {
                        if (button->wait_time < button->debounce_ticks) {
                            new_state(button_name, button, button->last_state);
                        } 
                    } else {
                        if (button->wait_time >= button->debounce_ticks) {
                            button->click_count += 1;
                            new_state(button_name, button, StateType::COUNT);
                        }
                    }
                    break;
                case StateType::COUNT:
                    if (button->button_pressed) {
                        new_state(button_name, button, StateType::PRESS);
                        button->start_time = esp_log_timestamp();
                    } else {
                        if (button->wait_time >= button->click_ticks || button->click_count == button->max_click_count) {
                            if (button->click_count == 1) {
                                ESP_LOGD(LOG_TAG, "%s click", button_name.c_str());
                                if (button->click_callback_func != nullptr) {
                                    try {
                                        button->click_callback_func(button->click_callback_func_args);
                                    } catch(const std::exception& e) {
                                        ESP_LOGE(LOG_TAG, "unexpected->%s", e.what());
                                    }
                                }
                            } else if (button->click_count == 2) {
                                ESP_LOGD(LOG_TAG, "%s double click", button_name.c_str());
                                if (button->double_click_callback_func != nullptr) {
                                    try {
                                        button->double_click_callback_func(button->double_click_callback_func_args);
                                    } catch(const std::exception& e) {
                                        ESP_LOGE(LOG_TAG, "unexpected->%s", e.what());
                                    }
                                }
                            } else {
                                ESP_LOGD(LOG_TAG, "%s multi click", button_name.c_str());
                                if (button->multi_click_callback_func != nullptr) {
                                    try {
                                        button->multi_click_callback_func(button->multi_click_callback_func_args);
                                    } catch(const std::exception& e) {
                                        ESP_LOGE(LOG_TAG, "unexpected->%s", e.what());
                                    }
                                }
                            }
                            reset(button);
                        }
                    }
                    break; 
                case StateType::LONG_PRESS:
                    if (button->button_pressed) {
                        ESP_LOGD(LOG_TAG, "%s during long press", button_name.c_str());
                        if (button->during_long_press_callback_func != nullptr) {
                            try {
                                button->during_long_press_callback_func(button->during_long_press_callback_func_args);
                            } catch(const std::exception& e) {
                                ESP_LOGE(LOG_TAG, "unexpected->%s", e.what());
                            }
                        }
                    } else {
                        new_state(button_name, button, StateType::LONG_PRESS_RELEASE);
                        button->start_time = esp_log_timestamp();
                    }
                    break;
                case StateType::LONG_PRESS_RELEASE:
                    if (button->button_pressed) {
                        if (button->wait_time < button->debounce_ticks) {
                            new_state(button_name, button, button->last_state);
                        }
                    } else {
                        if (button->wait_time >= button->debounce_ticks) {
                            ESP_LOGD(LOG_TAG, "%s long press stop", button_name.c_str());
                            if (button->long_press_stop_callback_func != nullptr) {
                                try {
                                    button->long_press_stop_callback_func(button->long_press_stop_callback_func_args);
                                } catch(const std::exception& e) {
                                    ESP_LOGE(LOG_TAG, "unexpected->%s", e.what());
                                }
                            }
                            reset(button);
                        }
                    }
                    break;
                default:
                    reset(button);
                    break;
            }
        }
        // 退出临界区
        xSemaphoreGive(mutex);
        //休眠至下一处理周期
        System::Sleep(10);
    }
}

void ButtonManager::Stop()
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
