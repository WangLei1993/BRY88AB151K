#include "cJSON.h"
#include "esp_log.h"

#include "utils.hpp"

#include "sgp30_config.hpp"

namespace cubestone_wang 
{

namespace sensor
{

using namespace utils;

const char *const SGP30Config::LOG_TAG = "SGP30_CONFIG";
const uint16_t SGP30Config::Default_CO2eq = 0;
const uint16_t SGP30Config::Default_TVOC = 0;

void SGP30Config::Reset()
{
    CO2eq = Default_CO2eq;
    TVOC = Default_TVOC;
}

std::string SGP30Config::Dump()
{
    cJSON *json_root = cJSON_CreateObject();
    cJSON_AddNumberToObject(json_root, "co2eq", CO2eq);	
    cJSON_AddNumberToObject(json_root, "tvoc", TVOC);	
    char *json_data = cJSON_PrintUnformatted(json_root);
    std::string result = std::string(json_data);
    cJSON_free(json_data);
    cJSON_Delete(json_root); 
    return result;
}

bool SGP30Config::Load(const char *const config_data)
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
    json_item = cJSON_GetObjectItem(json_root, "co2eq");
    if (NULL == json_item) {
        CO2eq = Default_CO2eq;
    } else if (cJSON_Number != json_item->type) {
        ESP_LOGE(LOG_TAG, "co2eq error");
        cJSON_Delete(json_root); 
        return false;
    } else {
        CO2eq = (uint16_t)json_item->valueint;
    }
    json_item = cJSON_GetObjectItem(json_root, "tvoc");
    if (NULL == json_item) {
        TVOC = Default_TVOC;
    } else if (cJSON_Number != json_item->type) {
        ESP_LOGE(LOG_TAG, "tvoc error");
        cJSON_Delete(json_root); 
        return false;
    } else {
        TVOC = (uint16_t)json_item->valueint;
    }
    cJSON_Delete(json_root); 
    return true;
}

bool SGP30Config::Load(const std::string &config_data)
{
    if ("" == config_data) {
        return Load(nullptr);
    } else {
        return Load(config_data.c_str());
    }
}

}

}
