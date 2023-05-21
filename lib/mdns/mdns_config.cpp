#include "string.h"

#include "cJSON.h"
#include "esp_log.h"
#include "esp_mac.h"

#include "utils.hpp"

#include "mdns_config.hpp"

namespace cubestone_wang 
{

namespace mdns
{

const char *const Config::LOG_TAG = "MDNS_CONFIG";

const char *const Config::default_instance_name = "";

Config::Config()
{
    uint8_t mac_address[6];
    ESP_ERROR_CHECK(esp_read_mac(mac_address, ESP_MAC_WIFI_STA));
    default_hostname = new char[13];
    memset(default_hostname, 0, sizeof(char) * 13); 
    sprintf(default_hostname, 
            "esp32_%02x%02x%02x", 
            mac_address[3],
            mac_address[4],
            mac_address[5]);
    MDNS::Service service;
}

Config::~Config()
{
    delete[] default_hostname;
}

void Config::Reset()
{
    Hostname = default_hostname;
    InstanceName = default_instance_name;
    services.clear();
    services = default_services;
}

std::string Config::Dump()
{
    cJSON *json_root = cJSON_CreateObject();
    cJSON *json_item, *json_sub_item, *json_child_item;
    cJSON_AddStringToObject(json_root, "hostname", Hostname.c_str());
    cJSON_AddStringToObject(json_root, "instance_name", InstanceName.c_str());
    if (0 < services.size()) {
        json_item = cJSON_AddArrayToObject(json_root, "services");
        for (auto index=0; index<services.size(); index++) {
            auto service = services[index];
            json_sub_item = cJSON_CreateObject();
            cJSON_AddStringToObject(json_sub_item, "instance_name", service.InstanceName.c_str());
            cJSON_AddStringToObject(json_sub_item, "type", service.Type.c_str());
            cJSON_AddStringToObject(json_sub_item, "proto", service.Proto.c_str());
            cJSON_AddNumberToObject(json_sub_item, "port", service.Port);
            if (0 < service.Txt.size()) {
                json_child_item = cJSON_AddObjectToObject(json_sub_item, "txt");
                for (auto sub_index=0; sub_index<service.Txt.size(); sub_index++) {
                    auto txt = service.Txt[sub_index];
                    cJSON_AddStringToObject(json_child_item, "key", txt.Key.c_str());
                    cJSON_AddStringToObject(json_child_item, "value", txt.Value.c_str());
                }
            }
            cJSON_AddItemToArray(json_item, json_sub_item);
        }
    }
    char *json_data = cJSON_PrintUnformatted(json_root);
    std::string result = std::string(json_data);
    cJSON_free(json_data);
    cJSON_Delete(json_root); 
    return result;
}

bool Config::Load(const char *const config_data)
{
    if (nullptr == config_data) {
        Reset();
        return true;
    }
    cJSON *json_root = cJSON_Parse(config_data);
    if (NULL == json_root) {   
        Reset();
        return true;
    }
    cJSON *json_item, *json_sub_item, *json_child_item, *json_four_item;
    json_item = cJSON_GetObjectItem(json_root, "hostname");
    if (NULL == json_item) {
        Hostname= default_hostname;
    } else if (cJSON_String != json_item->type) {
        ESP_LOGE(LOG_TAG, "hostname error");
        cJSON_Delete(json_root); 
        return false;
    } else {
        Hostname = json_item->valuestring;
    }
    json_item = cJSON_GetObjectItem(json_root, "instance_name");
    if (NULL == json_item) {
        InstanceName= default_instance_name;
    } else if (cJSON_String != json_item->type) {
        ESP_LOGE(LOG_TAG, "instance_name error");
        cJSON_Delete(json_root); 
        return false;
    } else {
        InstanceName = json_item->valuestring;
    }
    json_item = cJSON_GetObjectItem(json_root, "services");
    if (NULL == json_item) {
        // 不做任何操作
    } else if (cJSON_Array != json_item->type) {
        ESP_LOGE(LOG_TAG, "services error");
        cJSON_Delete(json_root); 
        return false;
    } else {
        for (uint8_t index=0; index<cJSON_GetArraySize(json_item); index++) {
            json_sub_item = cJSON_GetArrayItem(json_item, index);
            if (cJSON_Object != json_sub_item->type) {
                ESP_LOGE(LOG_TAG, "service %d error", index);
                cJSON_Delete(json_root); 
                return false;
            } else {
                MDNS::Service service;
                json_child_item = cJSON_GetObjectItem(json_sub_item, "instance_name");
                if (NULL == json_child_item) {
                    ESP_LOGE(LOG_TAG, "instance_name error");
                    cJSON_Delete(json_root); 
                    return false;
                } else if (cJSON_String != json_child_item->type) {
                    ESP_LOGE(LOG_TAG, "instance_name error");
                    cJSON_Delete(json_root); 
                    return false;
                } else {
                    service.InstanceName = json_child_item->valuestring;
                }
                json_child_item = cJSON_GetObjectItem(json_sub_item, "type");
                if (NULL == json_child_item) {
                    ESP_LOGE(LOG_TAG, "type error");
                    cJSON_Delete(json_root); 
                    return false;
                } else if (cJSON_String != json_child_item->type) {
                    ESP_LOGE(LOG_TAG, "type error");
                    cJSON_Delete(json_root); 
                    return false;
                } else {
                    service.Type = json_child_item->valuestring;
                }
                json_child_item = cJSON_GetObjectItem(json_sub_item, "proto");
                if (NULL == json_child_item) {
                    ESP_LOGE(LOG_TAG, "proto error");
                    cJSON_Delete(json_root); 
                    return false;
                } else if (cJSON_String != json_child_item->type) {
                    ESP_LOGE(LOG_TAG, "proto error");
                    cJSON_Delete(json_root); 
                    return false;
                } else {
                    service.Proto = json_child_item->valuestring;
                }
                json_child_item = cJSON_GetObjectItem(json_sub_item, "port");
                if (NULL == json_child_item) {
                    ESP_LOGE(LOG_TAG, "port error");
                    cJSON_Delete(json_root); 
                    return false;
                } else if (cJSON_Number != json_child_item->type) {
                    ESP_LOGE(LOG_TAG, "port error");
                    cJSON_Delete(json_root); 
                    return false;
                } else {
                    service.Port = (uint16_t)json_child_item->valueint;
                }
                json_child_item = cJSON_GetObjectItem(json_sub_item, "txt");
                if (NULL == json_child_item) {
                    // 不做任何操作
                } else if (cJSON_Object != json_child_item->type) {
                    ESP_LOGE(LOG_TAG, "txt error");
                    cJSON_Delete(json_root); 
                    return false;
                } else {
                    for (uint8_t sub_index=0; sub_index<cJSON_GetArraySize(json_child_item); sub_index++) {
                        MDNS::TxtItem txt_item;
                        json_four_item = cJSON_GetObjectItem(json_child_item, "key");
                        if (NULL == json_four_item) {
                            ESP_LOGE(LOG_TAG, "key error");
                            cJSON_Delete(json_root); 
                            return false;
                        } else if (cJSON_String != json_four_item->type) {
                            ESP_LOGE(LOG_TAG, "key error");
                            cJSON_Delete(json_root); 
                            return false;
                        } else {
                           txt_item.Key = json_four_item->valuestring;
                        }
                        json_four_item = cJSON_GetObjectItem(json_child_item, "value");
                        if (NULL == json_four_item) {
                            ESP_LOGE(LOG_TAG, "value error");
                            cJSON_Delete(json_root); 
                            return false;
                        } else if (cJSON_String != json_four_item->type) {
                            ESP_LOGE(LOG_TAG, "value error");
                            cJSON_Delete(json_root); 
                            return false;
                        } else {
                           txt_item.Value = json_four_item->valuestring;
                        }
                        service.Txt.push_back(txt_item);
                    }
                }
                services.push_back(service);
            }
        }
    }
    cJSON_Delete(json_root); 
    return true;
}

bool Config::Load(const std::string &config_data)
{
    if ("" == config_data) {
        return Load(nullptr);
    } else {
        return Load(config_data.c_str());
    }
}

}

}
