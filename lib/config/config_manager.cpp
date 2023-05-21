#include <string>

#include "esp_log.h"

#include "nonvolatile_storage.hpp"

#include "config_manager.hpp"

namespace cubestone_wang 
{

namespace config
{

using namespace nonvolatile_storage;

const char *const ConfigManager::LOG_TAG = "CONFIG_MANAGER";
const char *const ConfigManager::ns = "CONFIG";
SemaphoreHandle_t ConfigManager::mutex = xSemaphoreCreateRecursiveMutex();
std::map<std::string, BaseConfig *> ConfigManager::config_map;

bool ConfigManager::inner_reset(const std::string &name, bool debug_log_enable)
{
    bool result = true;
    auto iter = config_map.find(name);
    if (iter!=config_map.end()) {
       iter->second->Reset();
        if (true == debug_log_enable) {
            ESP_LOGD(LOG_TAG, "%s reset success", name.c_str());
        }
    } else {
        result = false;
        ESP_LOGE(LOG_TAG, "can't find %s config", name.c_str());
    }
    return result;
}

bool ConfigManager::inner_save(const std::string &name, bool debug_log_enable)
{
    bool result = true;
    auto iter = config_map.find(name);
    if (iter!=config_map.end()) {
        result = NonvolatileStorage::WriteString(ns, name.c_str(), iter->second->Dump());
        if (false == result) {
            ESP_LOGE(LOG_TAG, "%s save failed", name.c_str());
        } else {
            if (true == debug_log_enable) {
                ESP_LOGD(LOG_TAG, "%s save success", name.c_str());
            }
        }
    } else {
        result = false;
        ESP_LOGE(LOG_TAG, "can't find %s config", name.c_str());
    }
    
    return result;
}

bool ConfigManager::inner_load(const std::string &name, bool debug_log_enable)
{
    std::string config_data;
    bool result = true;
    auto iter = config_map.find(name);
    if (iter!=config_map.end()) {
        config_data = NonvolatileStorage::ReadString(ns, name);
        ESP_LOGD(LOG_TAG, "%s: %d %s", name.c_str(), config_data.length(), config_data.c_str());
        result = iter->second->Load(config_data);
        if (false == result) {
            ESP_LOGE(LOG_TAG, "%s load failed", name.c_str());
        } else {
            if (true == debug_log_enable) {
                ESP_LOGD(LOG_TAG, "%s load success", name.c_str());
            }
        }
    } else {
        result = false;
        ESP_LOGE(LOG_TAG, "can't find %s config", name.c_str());
    }
    return result;
}

bool ConfigManager::inner_run_func(InnerFunc_t inner_func, EndFunc_t end_func, const std::string &name)
{
    // 设置临界区
    xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    bool result = true;
    if ("" != name) {
        result = inner_func(name, true);
        if (false == result) {
            goto DONE;
        }
    } else {
        for (auto iter=config_map.begin(); iter!=config_map.end(); iter++) {
            result = inner_func(iter->first, false);
            if (false == result) {
                goto DONE;
            }
        }
        end_func();
    }
DONE:
    // 退出临界区
    xSemaphoreGiveRecursive(mutex);  
    return result;
}

bool ConfigManager::Reset(const std::string &name)
{
    return inner_run_func(inner_reset, 
                          [](){
                              ESP_LOGD(LOG_TAG, "all configs have been reset");
                          }, 
                          name);
}

bool ConfigManager::Save(const std::string &name)
{
    return inner_run_func(inner_save, 
                          [](){
                              ESP_LOGD(LOG_TAG, "all configs have been saved");
                          }, 
                          name);
}

bool ConfigManager::Load(const std::string &name)
{ 
    return inner_run_func(inner_load, 
                          [](){
                              ESP_LOGD(LOG_TAG, "all configs have been loaded");
                          }, 
                          name);
}

BaseConfig *ConfigManager::Get(const std::string &name)
{
    BaseConfig *config;
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    auto iter = config_map.find(name);
    if (iter != config_map.end()) {
        config = iter->second;
    } else {
        ESP_LOGE(LOG_TAG, "can't find %s config", name.c_str());
        config = nullptr;
    }
    // 退出临界区
    xSemaphoreGive(mutex);
    return config;
}

bool ConfigManager::Add(const std::string &name, BaseConfig *config, const bool &override)
{
    bool result;
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    auto iter = config_map.find(name);
    if (iter != config_map.end()) {
        if (true == override) {
            delete iter->second;
            iter->second = config;
            result = true;
        } else {
            ESP_LOGE(LOG_TAG, "%s config has been exist", name.c_str());
            result = false;
        }
    } else {
        config_map[name] = config;
        result = true;
    }
    // 退出临界区
    xSemaphoreGive(mutex);
    return result;
}

bool ConfigManager::Del(const std::string &name)
{
    bool result;
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    auto iter = config_map.find(name);
    if (iter != config_map.end()) {
        delete iter->second;
        iter->second = nullptr;
        config_map.erase(iter);
        result = true;
    } else {
        ESP_LOGE(LOG_TAG, "can't find %s config", name.c_str());
        result = false;
    }
    // 退出临界区
    xSemaphoreGive(mutex);
    return result;
}

}

}
