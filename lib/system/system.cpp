#include <algorithm>
#include "string.h"
#include "time.h"

#include "esp_chip_info.h"
#include "esp_private/esp_clk.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_flash.h"
#include "esp_ota_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "system.hpp"

namespace cubestone_wang 
{

namespace system 
{

const char *const System::LOG_TAG = "SYSTEM";

bool System::restart_flag = false;

SemaphoreHandle_t System::restart_mutex = xSemaphoreCreateMutex();

void System::LogHardwareInfo()
{   
    ESP_LOGI(LOG_TAG, "Hardware info:");
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    uint32_t flash_size;
    esp_flash_get_size(NULL, &flash_size);
    ESP_LOGI(LOG_TAG, "ESP32 chip with %d CPU cores, WiFi%s%s, revision %d",
             chip_info.cores,
             (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
             (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
             chip_info.revision);
    ESP_LOGI(LOG_TAG, "%luMB %s flash", 
             flash_size / (1024 * 1024),
             (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    ESP_LOGI(LOG_TAG, "CPU freq: %d mHz", esp_clk_cpu_freq()/1000/1000);
}

void System::LogSoftwareInfo()
{   
    ESP_LOGI(LOG_TAG, "Software info:");
    const esp_app_desc_t *app_desc = esp_app_get_description();
    ESP_LOGI(LOG_TAG, "Project name:     %s", app_desc->project_name);
    ESP_LOGI(LOG_TAG, "App version:      %s", app_desc->version);
    ESP_LOGI(LOG_TAG, "Compile time:     %s %s", app_desc->date, app_desc->time);
    char buf[17];
    esp_app_get_elf_sha256(buf, sizeof(buf));
    ESP_LOGI(LOG_TAG, "ELF file SHA256:  %s...", buf);
    ESP_LOGI(LOG_TAG, "ESP-IDF:          %s", app_desc->idf_ver);
}

void System::Sleep(const uint32_t milliseconds)
{
    vTaskDelay(milliseconds / portTICK_PERIOD_MS);
}

void System:: Restart(const char *const reason, const uint32_t delay)
{
    // 判断是否已经执行过 
    // 设置临界区
    xSemaphoreTake(restart_mutex, portMAX_DELAY);
    if (restart_flag == true) {
        ESP_LOGI(LOG_TAG, "restart have been set");
        // 退出临界区
        xSemaphoreGive(restart_mutex);
        return;
    }
    restart_flag = true;
    // 退出临界区
    xSemaphoreGive(restart_mutex);
    if (reason != nullptr) {
        ESP_LOGI(LOG_TAG, "restart after %lu s, the reason is %s", delay, reason);
    } else {
        ESP_LOGI(LOG_TAG, "restart after %lu s with no reason", delay);
    }
    if (0 == delay) {
        ESP_LOGI(LOG_TAG, "restart now");
        esp_restart();
    } else {
        xTaskCreate([](void *delay) {
                        for (uint32_t i = (uint32_t)delay; i > 0; i--) {
                            ESP_LOGI(LOG_TAG, "restart in %lu s...", i);
                            Sleep(1000);
                        }
                        ESP_LOGI(LOG_TAG, "restart now");
                        esp_restart();
                    }, 
                    "restart", 
                    4096, 
                    (void *)delay, 
                    configMAX_PRIORITIES,
                    NULL);
    }
}

uint32_t System::GetCurrentFreeHeapSize()
{
    esp_get_free_internal_heap_size();
    return esp_get_free_heap_size();
}

uint32_t System::GetCurrentMinimumFreeHeapSize()
{
    return esp_get_minimum_free_heap_size();
}

System::TaskInfoSummary System::GetCurrentTaskInfoSummary()
{
    auto task_info_summary = TaskInfoSummary();
    UBaseType_t task_count = uxTaskGetNumberOfTasks();
    TaskStatus_t *task_status_array = new TaskStatus_t[task_count];
    uint32_t total_run_time;
    task_count = uxTaskGetSystemState(task_status_array, task_count, &total_run_time);
    task_info_summary.TotalRuntime = total_run_time;
    TaskInfo task_info; 
    for (auto index=0; index<task_count; index++) {
        task_info.Id = (uint32_t)task_status_array[index].xTaskNumber;
        task_info.Name = std::string(task_status_array[index].pcTaskName);
        task_info.Priority = (uint32_t)task_status_array[index].uxCurrentPriority;
        task_info.State = task_status_array[index].eCurrentState;
        task_info.Runtime = (uint32_t)task_status_array[index].ulRunTimeCounter;
        task_info.StackHighWaterMark = (uint32_t)task_status_array[index].usStackHighWaterMark;
        task_info.CoreID = (int32_t)task_status_array[index].xCoreID;
        if (task_info.CoreID == tskNO_AFFINITY) {
            task_info.CoreID = -1;
        }
        task_info_summary.TaskInfoList.push_back(task_info);
    }
    delete[] task_status_array;
    std::sort(task_info_summary.TaskInfoList.begin(),
              task_info_summary.TaskInfoList.end(),
              [](const TaskInfo &a, const TaskInfo &b){ return a.Id < b.Id;});
    return task_info_summary;
}

time_t System::GetCurrentTimestamp()
{
    return time(NULL);
}

struct tm System::GetCurrentTime()
{
    struct tm time_info;
    time_t now = time(NULL);
    localtime_r(&now, &time_info);
    return time_info;
}

std::string System::GetCurrentTimeStandardString()
{
    time_t now = time(NULL);
    struct tm time_info;
    localtime_r(&now, &time_info);
    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &time_info);
    return std::string(strftime_buf);
}

time_t System::GetStartupTimestamp()
{   
    // 获取时间频率需小于49天，否则会有问题
    static time_t last_get_startup_timestamp = 0;
    static time_t total_seconds = 0;
    static time_t remaining_millisecond = 0;
    time_t startup_timestamp = esp_log_timestamp();
    time_t duration;
    if (startup_timestamp > last_get_startup_timestamp) {
        duration = startup_timestamp - last_get_startup_timestamp;
        total_seconds += duration / 1000;
        remaining_millisecond += duration % 1000;
        total_seconds += remaining_millisecond / 1000;
        remaining_millisecond %= 1000;
    } else {
        duration = UINT32_MAX - last_get_startup_timestamp;
        total_seconds += duration / 1000;
        remaining_millisecond += duration % 1000;
        total_seconds += remaining_millisecond / 1000;
        remaining_millisecond %= 1000;
        duration = startup_timestamp;
        total_seconds += duration / 1000;
        remaining_millisecond += duration % 1000;
        total_seconds += remaining_millisecond / 1000;
        remaining_millisecond %= 1000;
    }
    last_get_startup_timestamp = startup_timestamp;
    return total_seconds;
}

std::string System::GetStartupTimeString()
{
    time_t timestamp = GetStartupTimestamp();
    uint32_t days, hours, minutes, seconds;
    char buffer[64];
    memset(buffer, 0, sizeof(buffer));
    days = timestamp / (24*60*60);
    timestamp %= (24*60*60);
    hours = timestamp / (60*60);
    timestamp %= (60*60);
    minutes = timestamp / 60;
    timestamp %= 60;
    seconds = timestamp;
    if (0 == days) {
        sprintf(buffer, 
                "%02lu:%02lu:%02lu",
                hours,
                minutes,
                seconds);
    } else if (1 == days) {
        sprintf(buffer, 
                "1 day %02lu:%02lu:%02lu",
                hours,
                minutes,
                seconds);
    } else {
        sprintf(buffer, 
                "%lu days %02lu:%02lu:%02lu",
                days, 
                hours,
                minutes,
                seconds);
    }
    return std::string(buffer);
}

bool System::DomainToIPAddress(const std::string &domain, ip_addr_t *ip)
{
    // parse IP address
    struct sockaddr_in6 sock_addr6;
    memset(ip, 0, sizeof(ip_addr_t));
    if (inet_pton(AF_INET6, domain.c_str(), &sock_addr6.sin6_addr) == 1) {
        /* convert ip6 string to ip6 address */
        ipaddr_aton(domain.c_str(), ip);
    } else {
        struct addrinfo hint;
        struct addrinfo *res = NULL;
        memset(&hint, 0, sizeof(addrinfo));
        /* convert ip4 string or domain to ip4 or ip6 address */
        if (getaddrinfo(domain.c_str(), NULL, &hint, &res) != 0) {
            ESP_LOGW(LOG_TAG, "unknown domain %s", domain.c_str());
            return false;
        }
        if (res->ai_family == AF_INET) {
            struct in_addr addr4 = ((struct sockaddr_in *) (res->ai_addr))->sin_addr;
            inet_addr_to_ip4addr(ip_2_ip4(ip), &addr4);
        } else {
            struct in6_addr addr6 = ((struct sockaddr_in6 *) (res->ai_addr))->sin6_addr;
            inet6_addr_to_ip6addr(ip_2_ip6(ip), &addr6);
        }
        freeaddrinfo(res);
    }
    return true;
}

}

}
