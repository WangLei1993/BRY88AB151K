#define SNTP_STARTUP_DELAY 0
#include "esp_log.h"
#include "esp_sntp.h"

#include "system.hpp"

#include "ntp.hpp"

namespace cubestone_wang 
{

namespace ntp 
{

using namespace system;

const char *const NTP::LOG_TAG = "NTP";

bool NTP::start_flag = false;

SemaphoreHandle_t NTP::mutex = xSemaphoreCreateMutex();

std::vector<std::string *> NTP::server_name_list;

bool NTP::Start(const std::vector<std::string> &server_name_list, const uint32_t wait_time)
{
    // 设置临界区
    bool result = false;
    sntp_sync_status_t status;
    uint32_t current_time;
    uint8_t index;
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (start_flag == true) {
        ESP_LOGI(LOG_TAG, "this has been started");
        result = false;
        goto DONE;
    }
    if (SNTP_MAX_SERVERS < server_name_list.size())
    {
        ESP_LOGE(LOG_TAG, "server count must <= CONFIG_LWIP_SNTP_MAX_SERVERS");
        result = false;
        goto DONE;
    }
    for (index=0; index<server_name_list.size(); index++) {
        NTP::server_name_list.push_back(new std::string(server_name_list[index]));
        sntp_setservername(index, NTP::server_name_list[index]->c_str());
    }
    start_flag = true;
    // 将时区设置为中国标准时间
    setenv("TZ", "CST-8", 1);
    tzset();
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_init();
    current_time = esp_log_timestamp();
    while (true) { 
        status = sntp_get_sync_status();
        if (SNTP_SYNC_STATUS_COMPLETED != status) {
            if (esp_log_timestamp() > (current_time + wait_time * 1000)) {
                result = false;
                ESP_LOGE(LOG_TAG, "sync failed");
                break;
            } else {
                System::Sleep(100);
            }
        } else {
            result = true;
            ESP_LOGD(LOG_TAG, "sync done");
            break;
        }
    }
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
DONE:
    // 退出临界区
    xSemaphoreGive(mutex);
    if (!result) {
        NTP::Stop();
    }
    return result;
}

void NTP::Stop()
{
    uint8_t index;
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (start_flag == false) { 
        ESP_LOGD(LOG_TAG, "this has been stopped");
        goto DONE;
    }
    sntp_stop();
    for (index=0; index<NTP::server_name_list.size(); index++) {
        sntp_setservername(index, NULL);
        delete NTP::server_name_list[index];
        NTP::server_name_list[index] = nullptr;
    }
    NTP::server_name_list.clear();
DONE:
    start_flag = false;
    // 退出临界区
    xSemaphoreGive(mutex);
    return;
}

}

}
