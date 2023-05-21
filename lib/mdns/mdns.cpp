#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "mdns.h"
#include "event_loop.hpp"

#include "mdns.hpp"

namespace cubestone_wang 
{

namespace mdns
{

using namespace event_loop;

const char *const MDNS::LOG_TAG = "MDNS";

bool MDNS::start_flag = false;

SemaphoreHandle_t MDNS::mutex = xSemaphoreCreateMutex();

bool MDNS::Start(const std::string &hostname, 
                 const std::string &instance_name, 
                 const std::vector<Service> &services)
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
    EventLoop::Init();
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set(hostname.c_str()));
    if (0 < instance_name.length()) {
        ESP_ERROR_CHECK(mdns_instance_name_set(instance_name.c_str()));
    }
    Service service;
    for (uint32_t index=0; index<services.size(); index++) {
        service = services[index];
        ESP_ERROR_CHECK(mdns_service_add(NULL,
                                         service.Type.c_str(),
                                         service.Proto.c_str(),
                                         service.Port,
                                         NULL, 
                                         0));
        for (uint32_t sub_index=0; sub_index<service.Txt.size(); sub_index++) {
            ESP_ERROR_CHECK(mdns_service_txt_item_set(service.Type.c_str(),
                                                      service.Proto.c_str(),
                                                      service.Txt[sub_index].Key.c_str(),
                                                      service.Txt[sub_index].Key.c_str()));
        }
        if (0 < service.InstanceName.length()) {
            ESP_ERROR_CHECK(mdns_service_instance_name_set(service.Type.c_str(),
                                                           service.Proto.c_str(),
                                                           service.InstanceName.c_str()));
        }
    }
    start_flag = true;
    // 退出临界区
    xSemaphoreGive(mutex);
    return true;
}

void MDNS::Stop()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (start_flag == false) { 
        ESP_LOGD(LOG_TAG, "this has been stopped");
        goto DONE;
    }
    mdns_free();
DONE:
    start_flag = false;
    // 退出临界区
    xSemaphoreGive(mutex);
    return;
}

}

}
