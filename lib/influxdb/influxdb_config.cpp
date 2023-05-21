#include "cJSON.h"
#include "esp_log.h"

#include "utils.hpp"

#include "influxdb_config.hpp"

namespace cubestone_wang 
{

namespace influxdb
{

using namespace utils;

const char *const Config::LOG_TAG = "INFLUXDB_CONFIG";
const uint16_t Config::default_port = 8086;
const std::string Config::default_org = "default";
const std::string Config::default_bucket = "default";
const uint8_t Config::default_timeout = 5;

void Config::Reset()
{
    Port = default_port;
    Org = default_org;
    Timeout = default_timeout;
}

std::string Config::Dump()
{
    cJSON *json_root = cJSON_CreateObject();
    cJSON_AddStringToObject(json_root, "host", Host.c_str());
    cJSON_AddNumberToObject(json_root, "port", Port);
    cJSON_AddStringToObject(json_root, "token", Token.c_str());
    cJSON_AddStringToObject(json_root, "org", Org.c_str());
    cJSON_AddStringToObject(json_root, "bucket", Bucket.c_str());
    cJSON_AddNumberToObject(json_root, "timeout", Timeout);
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
    cJSON *json_item;
    json_item = cJSON_GetObjectItem(json_root, "host");
    if (NULL == json_item || cJSON_String != json_item->type) {
        ESP_LOGE(LOG_TAG, "host error");
        cJSON_Delete(json_root); 
        return false;
    } else {
        Host = json_item->valuestring;
    }
    json_item = cJSON_GetObjectItem(json_root, "port");
    if (NULL == json_item) {
        Port = default_port;
    } else if (cJSON_Number != json_item->type) {
        ESP_LOGE(LOG_TAG, "port error");
        cJSON_Delete(json_root); 
        return false;
    } else {
        Port = (uint16_t)json_item->valueint;
    }
    json_item = cJSON_GetObjectItem(json_root, "token");
    if (NULL == json_item || cJSON_String != json_item->type) {
        ESP_LOGE(LOG_TAG, "token error");
        cJSON_Delete(json_root); 
        return false;
    } else {
        Token = json_item->valuestring;
    }
    json_item = cJSON_GetObjectItem(json_root, "org");
    if (NULL == json_item) {
        Org = default_org;
    } else if (cJSON_String != json_item->type) {
        ESP_LOGE(LOG_TAG, "org error");
        cJSON_Delete(json_root); 
        return false;
    } else {
        Org = json_item->valuestring;
    }
    json_item = cJSON_GetObjectItem(json_root, "bucket");
    if (NULL == json_item) {
        Bucket = default_bucket;
    } else if (cJSON_String != json_item->type) {
        ESP_LOGE(LOG_TAG, "bucket error");
        cJSON_Delete(json_root); 
        return false;
    } else {
        Bucket = json_item->valuestring;
    }
    json_item = cJSON_GetObjectItem(json_root, "timeout");
    if (NULL == json_item) {
        Timeout = default_timeout;
    } else if (cJSON_Number != json_item->type) {
        ESP_LOGE(LOG_TAG, "timeout error");
        cJSON_Delete(json_root); 
        return false;
    } else {
        Timeout = (uint8_t)json_item->valueint;
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
