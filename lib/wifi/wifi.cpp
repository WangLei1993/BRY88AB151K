#include "string.h"

#include "esp_log.h"
#include "esp_system.h"

#include "event_loop.hpp"

#include "wifi.hpp"

namespace cubestone_wang 
{

namespace wifi 
{

using namespace event_loop;

const char *const WiFi::LOG_TAG = "WIFI";

bool WiFi::start_flag = false;

SemaphoreHandle_t WiFi::mutex = xSemaphoreCreateRecursiveMutex();

esp_netif_t *WiFi::netif = nullptr;

const EventBits_t WiFi::WIFI_SUCCESS_BIT = BIT0;
const EventBits_t WiFi::WIFI_FAILURE_BIT = BIT1;

wifi_mode_t WiFi::current_mode = WIFI_MODE_NULL;

esp_event_handler_instance_t WiFi::event_handler_instance = nullptr;

EventGroupHandle_t WiFi::event_group = nullptr;

uint32_t WiFi::retry_count = 0;
const uint32_t WiFi::MAX_RETRY_COUNT = 10;

void WiFi::event_handler(void *args, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_ERROR_CHECK(esp_wifi_connect());
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* disconnected = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGD(LOG_TAG, 
                 "disconnect ssid: %s, ssid len: %d, reason: %d", 
                 disconnected->ssid,
                 disconnected->ssid_len,
                 disconnected->reason);
        // 仅在初始启动时有最大重连次数限制，运行中将不断尝试重连
        if (event_group != nullptr) {
            if (retry_count <MAX_RETRY_COUNT) {
                ESP_ERROR_CHECK(esp_wifi_connect());
                retry_count++;
            } else {
                if (event_group != nullptr) {
                    xEventGroupSetBits(event_group, WIFI_FAILURE_BIT);
                }
            }
        } else {
            if (true == start_flag) {
                esp_wifi_connect();
            }
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(LOG_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        if (nullptr != event_group) {
            xEventGroupSetBits(event_group, WIFI_SUCCESS_BIT);
        }
    }
}   

bool WiFi::Start(const wifi_mode_t &mode, const std::string &ssid, const std::string &password, const std::string &hostname)
{
    bool result = true;
    EventBits_t event_bits;
    wifi_config_t wifi_config = {};
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    // 设置临界区
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    // 判断是否已经启动 
    if (true == start_flag) {
        ESP_LOGI(LOG_TAG, "this has been started");
        result = false;
        goto DONE;
    }
    EventLoop::Init();
    ESP_ERROR_CHECK(esp_netif_init());
    current_mode = mode;
    if (WIFI_MODE_STA == mode) {
        event_group = xEventGroupCreate();
        netif = esp_netif_create_default_wifi_sta();
        if (0 < hostname.length()) {
            ESP_ERROR_CHECK(esp_netif_set_hostname(netif, hostname.c_str()));
        }
        ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
        esp_event_handler_instance_t instance_got_ip;
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                            ESP_EVENT_ANY_ID,
                                                            &event_handler,
                                                            nullptr,
                                                            &event_handler_instance));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                            IP_EVENT_STA_GOT_IP,
                                                            &event_handler,
                                                            nullptr,
                                                            &instance_got_ip));
        wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
        wifi_config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        wifi_config.sta.bssid_set = false;
        wifi_config.sta.channel = 0;
        memset(wifi_config.sta.ssid, 0, 32); 
        memcpy(wifi_config.sta.ssid, ssid.c_str(), ssid.length());
        memset(wifi_config.sta.password, 0, 64); 
        memcpy(wifi_config.sta.password, password.c_str(), password.length());
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
        event_bits = xEventGroupWaitBits(event_group,
                                         WIFI_SUCCESS_BIT | WIFI_FAILURE_BIT,
                                         pdFALSE,
                                         pdFALSE,
                                         portMAX_DELAY);
        ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
        vEventGroupDelete(event_group);  
        event_group = nullptr;
        if (event_bits & WIFI_SUCCESS_BIT) {
            ESP_LOGD(LOG_TAG, "connected to ap");
        } else if (event_bits & WIFI_FAILURE_BIT) {
            result = false;
            ESP_LOGD(LOG_TAG, "failed to connect to ap");
        } else {
            result = false;
            ESP_LOGE(LOG_TAG, "unexpected event");
        }
        WiFi::start_flag = true;
    } else {
        netif = esp_netif_create_default_wifi_ap();
        if (0 < hostname.length()) {
            ESP_ERROR_CHECK(esp_netif_set_hostname(netif, hostname.c_str()));
        }
        ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
        wifi_config.ap.ssid_len = 0;
        wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
        wifi_config.ap.ssid_hidden = 0;
        wifi_config.ap.channel = 6;
        wifi_config.ap.max_connection = 2;
        memset(wifi_config.ap.ssid, 0, ssid.length()+1); 
        memcpy(wifi_config.ap.ssid, ssid.c_str(), ssid.length());
        memset(wifi_config.ap.password, 0, password.length()+1); 
        memcpy(wifi_config.ap.password, password.c_str(), password.length());
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
        ESP_ERROR_CHECK(esp_wifi_start());
        ESP_LOGD(LOG_TAG, "ap start");
        WiFi::start_flag = true;
    }
DONE:
    // 退出临界区
    xSemaphoreGiveRecursive(mutex);
    if (false == result) {
        WiFi::Stop();
    }
    return result;
}

void WiFi::Stop()
{
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    if (false == start_flag) { 
        ESP_LOGD(LOG_TAG, "this has been stopped");
        goto DONE;
    }
    start_flag = false;
    if (WIFI_MODE_STA == current_mode) {
        esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler_instance);
        event_handler_instance = nullptr;
        esp_wifi_disconnect();
    }
    esp_wifi_stop();
    esp_wifi_deinit();
    esp_wifi_clear_default_wifi_driver_and_handlers(netif);
    esp_netif_destroy(netif);
    netif = nullptr;
    current_mode = WIFI_MODE_NULL;
    // ESP_ERROR_CHECK(esp_netif_deinit());
DONE:
    // 退出临界区
    xSemaphoreGiveRecursive(mutex);
    return;
}

esp_netif_t *WiFi::GetNetif()
{
    return netif;
}

}

}
