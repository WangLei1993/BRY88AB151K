#include "cJSON.h"
#include "esp_log.h"

#include "utils.hpp"

#include "ntp_config.hpp"

namespace cubestone_wang 
{

namespace ntp
{

const char *const Config::LOG_TAG = "NTP_CONFIG";

void Config::Reset()
{
    ServerNameList.clear();
    ServerNameList.push_back("s1a.time.edu.cn");
    ServerNameList.push_back("ntp.sjtu.edu.cn");
    ServerNameList.push_back("ntp1.aliyun.com");
}

std::string Config::Dump()
{
    cJSON *json_root = cJSON_CreateObject();
    cJSON *json_item;
    json_item = cJSON_AddArrayToObject(json_root, "server_name_list");
    for (auto index=0; index<ServerNameList.size(); index++) {
        cJSON_AddItemToArray(json_item, cJSON_CreateString(ServerNameList[index].c_str()));
    }
    char *json_data = cJSON_PrintUnformatted(json_root);
    std::string result = std::string(json_data);
    cJSON_free(json_data);
    cJSON_Delete(json_root); 
    return result;
}

bool Config::Load(const char *const config_data)
{
    ServerNameList.clear();
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
    json_item = cJSON_GetObjectItem(json_root, "server_name_list");
    if (NULL == json_item) {
        Reset();
    } else if (cJSON_Array != json_item->type) {
        ESP_LOGE(LOG_TAG, "server_name_list error");
        cJSON_Delete(json_root); 
        return false;
    } else {
        for (uint8_t index=0; index<cJSON_GetArraySize(json_item); index++) {
            json_sub_item = cJSON_GetArrayItem(json_item, index);
            if (cJSON_String != json_sub_item->type) {
                ESP_LOGE(LOG_TAG, "server_name_list %d error", index);
                cJSON_Delete(json_root); 
                return false;
            } else {
                ServerNameList.push_back(json_sub_item->valuestring);   
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
