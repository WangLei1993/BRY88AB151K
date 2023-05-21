#include "string.h"

#include "cJSON.h"
#include "esp_log.h"
#include "esp_mac.h"

#include "wifi_config.hpp"

namespace cubestone_wang 
{

namespace wifi
{

const char *const Config::LOG_TAG = "WIFI_CONFIG";
const char *const Config::default_ap_password = "12345678";
const char *const Config::default_sta_ssid = "";
const char *const Config::default_sta_password = "";

Config::Config()
{
    uint8_t mac_address[6];
    default_ap_ssid = new char[13];
    default_ap_hostname = new char[13];
    default_sta_hostname = new char[13];
    memset(default_ap_ssid, 0, sizeof(char) * 13); 
    memset(default_ap_hostname, 0, sizeof(char) * 13); 
    memset(default_sta_hostname, 0, sizeof(char) * 13); 
    ESP_ERROR_CHECK(esp_read_mac(mac_address, ESP_MAC_WIFI_SOFTAP));
    sprintf(default_ap_ssid,  "esp32_%02x%02x%02x", mac_address[3], mac_address[4], mac_address[5]);
    sprintf(default_ap_hostname, "esp32_%02x%02x%02x", mac_address[3], mac_address[4], mac_address[5]);
    ESP_ERROR_CHECK(esp_read_mac(mac_address, ESP_MAC_WIFI_STA));
    sprintf(default_sta_hostname, "esp32_%02x%02x%02x", mac_address[3], mac_address[4], mac_address[5]);
}

Config::~Config()
{
    delete default_ap_ssid;
    default_ap_ssid = nullptr;
    delete default_ap_hostname;
    default_ap_hostname = nullptr;
    delete default_sta_hostname;
    default_sta_hostname = nullptr;
}

void Config::Reset()
{   
    Mode = WIFI_MODE_STA;
    AP.SSID = default_ap_ssid;
    AP.Password = default_ap_password;
    AP.Hostname = default_ap_hostname;
    STA.SSID = default_sta_ssid;
    STA.Password = default_sta_password;
    STA.Hostname = default_sta_hostname;
}

std::string Config::Dump()
{
    cJSON *json_root = cJSON_CreateObject();
    cJSON *json_item;
    cJSON_AddNumberToObject(json_root, "mode", Mode);	
    json_item = cJSON_AddObjectToObject(json_root, "ap");
    cJSON_AddStringToObject(json_item, "ssid", AP.SSID.c_str());
    cJSON_AddStringToObject(json_item, "password", AP.Password.c_str());
    cJSON_AddStringToObject(json_item, "hostname", AP.Hostname.c_str());
    json_item = cJSON_AddObjectToObject(json_root, "sta");	
    cJSON_AddStringToObject(json_item, "ssid", STA.SSID.c_str());
    cJSON_AddStringToObject(json_item, "password", STA.Password.c_str());   
    cJSON_AddStringToObject(json_item, "hostname", STA.Hostname.c_str());
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
    cJSON *json_item, *json_sub_item;
    json_item = cJSON_GetObjectItem(json_root, "mode");
    if (NULL == json_item) {
        Mode = WIFI_MODE_AP;
    } else if (cJSON_Number != json_item->type) {
        ESP_LOGE(LOG_TAG, "mode error");
        cJSON_Delete(json_root); 
        return false;
    } else if (WIFI_MODE_AP == json_item->valueint) {
        Mode = WIFI_MODE_AP;
    } else if (WIFI_MODE_STA == json_item->valueint) {
        Mode = WIFI_MODE_STA;
    } else {
        ESP_LOGE(LOG_TAG, "mode error");
        cJSON_Delete(json_root); 
        return false;
    }
    json_item = cJSON_GetObjectItem(json_root, "ap");
    if (NULL == json_item) {
        
        AP.SSID = default_ap_ssid;
        AP.Password = default_ap_password;
    } else if (cJSON_Object != json_item->type) {
        ESP_LOGE(LOG_TAG, "ap error");
        cJSON_Delete(json_root); 
        return false;
    } else {
        json_sub_item = cJSON_GetObjectItem(json_item, "ssid");
        if (NULL == json_sub_item) {
            AP.SSID = default_ap_ssid;
        } else if (cJSON_String != json_sub_item->type) {
            ESP_LOGE(LOG_TAG, "ap ssid error");
            cJSON_Delete(json_root); 
            return false;
        } else {
            AP.SSID = json_sub_item->valuestring;
        }
        json_sub_item = cJSON_GetObjectItem(json_item, "password");
        if (NULL == json_sub_item) {
            AP.Password = default_ap_password;
        } else if (cJSON_String != json_sub_item->type) {
            ESP_LOGE(LOG_TAG, "ap password error");
            cJSON_Delete(json_root); 
            return false;
        } else {
            AP.Password = json_sub_item->valuestring;
        }
        json_sub_item = cJSON_GetObjectItem(json_item, "hostname");
        if (NULL == json_sub_item) {
            AP.Hostname = default_ap_hostname;
        } else if (cJSON_String != json_sub_item->type) {
            ESP_LOGE(LOG_TAG, "ap hostname error");
            cJSON_Delete(json_root); 
            return false;
        } else {
            AP.Hostname = json_sub_item->valuestring;
        }
    }
    json_item = cJSON_GetObjectItem(json_root, "sta");
    if (NULL == json_item) {
        STA.SSID = default_sta_ssid;
        STA.Password = default_sta_password;
    } else if (cJSON_Object != json_item->type) {
        ESP_LOGE(LOG_TAG, "sta error");
        cJSON_Delete(json_root); 
        return false;
    } else {
        json_sub_item = cJSON_GetObjectItem(json_item, "ssid");
        if (NULL == json_sub_item) {
            STA.SSID = default_sta_ssid;
        } else if (cJSON_String != json_sub_item->type) {
            ESP_LOGE(LOG_TAG, "sta ssid error");
            cJSON_Delete(json_root); 
            return false;
        } else {
            STA.SSID = json_sub_item->valuestring;
        }
        json_sub_item = cJSON_GetObjectItem(json_item, "password");
        if (NULL == json_sub_item) {
            STA.Password = default_sta_password;
        } else if (cJSON_String != json_sub_item->type) {
            ESP_LOGE(LOG_TAG, "sta password error");
            cJSON_Delete(json_root); 
            return false;
        } else {
            STA.Password = json_sub_item->valuestring;
        }
        json_sub_item = cJSON_GetObjectItem(json_item, "hostname");
        if (NULL == json_sub_item) {
            STA.Hostname = default_sta_hostname;
        } else if (cJSON_String != json_sub_item->type) {
            ESP_LOGE(LOG_TAG, "sta hostname error");
            cJSON_Delete(json_root); 
            return false;
        } else {
            STA.Hostname = json_sub_item->valuestring;
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
