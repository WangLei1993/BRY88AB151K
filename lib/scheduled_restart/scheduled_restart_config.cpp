#include "cJSON.h"
#include "esp_log.h"

#include "system.hpp"

#include "scheduled_restart_config.hpp"

namespace cubestone_wang 
{

namespace scheduled_restart
{

using namespace system;

const char *const Config::LOG_TAG = "SCHEDULED_RESTART_CONFIG";

const ScheduledRestart::DayType Config::default_day_type = ScheduledRestart::DayType::RESTART_DAY_TYPE_EVERY_DAY; 
const uint8_t Config::default_hour = 4; 
const uint8_t Config::default_minute = 0; 

void Config::Reset()
{
    DayType = default_day_type;
    Hour = default_hour;
    Minute = default_minute;
}

std::string Config::Dump()
{
    cJSON *json_root = cJSON_CreateObject();
    cJSON_AddNumberToObject(json_root, "day_type", static_cast<uint32_t>(DayType));
    cJSON_AddNumberToObject(json_root, "hour", Hour);
    cJSON_AddNumberToObject(json_root, "minute", Minute);	
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
    json_item = cJSON_GetObjectItem(json_root, "day_type");
    if (NULL == json_item) {
        DayType = default_day_type;
    } else if (cJSON_Number != json_item->type) {
        ESP_LOGE(LOG_TAG, "cron error");
        cJSON_Delete(json_root); 
        return false;
    } else {
        DayType = (ScheduledRestart::DayType)(json_item->valueint);
    }
    json_item = cJSON_GetObjectItem(json_root, "hour");
    if (NULL == json_item) {
        Hour = default_hour;
    } else if (cJSON_Number != json_item->type) {
        ESP_LOGE(LOG_TAG, "hour error");
        cJSON_Delete(json_root); 
        return false;
    } else {
        Hour = (uint8_t)(json_item->valueint);
    }
    json_item = cJSON_GetObjectItem(json_root, "minute");
    if (NULL == json_item) {
        Minute = default_minute;
    } else if (cJSON_Number != json_item->type) {
        ESP_LOGE(LOG_TAG, "minute error");
        cJSON_Delete(json_root); 
        return false;
    } else {
        Minute = (uint8_t)(json_item->valueint);
    }
    
    cJSON_Delete(json_root); 
    return true;
}

bool Config::Load(const std::string &config_data)
{
    if ("" == config_data) {
        return this->Load(nullptr);
    } else {
        return this->Load(config_data.c_str());
    }
}

}

}
