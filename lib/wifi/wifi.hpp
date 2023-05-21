#ifndef _wifi_hpp_
#define _wifi_hpp_

#include <string>

#include "esp_event.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

namespace cubestone_wang 
{

namespace wifi 
{

class WiFi
{
    public:
        // 日志标签
        static const char *const LOG_TAG;
        static bool Start(const wifi_mode_t &mode, const std::string &ssid, const std::string &password, const std::string &hostname="");
        static void Stop();
        static esp_netif_t *GetNetif();
    private:
        static SemaphoreHandle_t mutex;
        static bool start_flag;
        static void event_handler(void *args, esp_event_base_t event_base, int32_t event_id, void* event_data);
        static esp_netif_t *netif;
        static const EventBits_t WIFI_SUCCESS_BIT;
        static const EventBits_t WIFI_FAILURE_BIT;
        static wifi_mode_t current_mode;
        static esp_event_handler_instance_t event_handler_instance;
        static uint32_t retry_count;
        static const uint32_t MAX_RETRY_COUNT;
        static EventGroupHandle_t event_group;
};

}

}

#endif // _wifi_hpp_
