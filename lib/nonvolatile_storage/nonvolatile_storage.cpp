#include "string.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "nonvolatile_storage.hpp"

namespace cubestone_wang 
{

namespace nonvolatile_storage 
{

const char *const NonvolatileStorage::LOG_TAG = "NVS";

SemaphoreHandle_t NonvolatileStorage::mutex = xSemaphoreCreateMutex();

bool NonvolatileStorage::init_flag = false;

void NonvolatileStorage::Init() 
{
    // 判断是否已经执行过 
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (init_flag == true) {
        ESP_LOGD(LOG_TAG, " it have been inited");
        // 退出临界区
        xSemaphoreGive(mutex);return;
    }
    init_flag = true;
    // 初始化数据
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGD(LOG_TAG, "erase flash");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    // 退出临界区
    xSemaphoreGive(mutex);
    return;
}

std::string NonvolatileStorage::ReadString(const std::string &ns, const std::string &key)
{
    nvs_handle_t handle;
    std::string result;
    uint32_t buffer_tmp_length;
    size_t buffer_length;
    char *buffer;
    auto key_len = key + "_l";
    esp_err_t err;
    NonvolatileStorage::Init();
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    ESP_ERROR_CHECK(nvs_open(ns.c_str(), NVS_READWRITE, &handle));
    err = nvs_get_u32(handle, key_len.c_str(), &buffer_tmp_length);
    if (ESP_OK != err) {
        if (ESP_ERR_NVS_NOT_FOUND != err) {
            ESP_LOGE(NonvolatileStorage::LOG_TAG, 
                     "error code->%d",
                     err);
        }
        goto DONE;
    }
    buffer_length = (size_t)buffer_tmp_length;
    ESP_LOGD(LOG_TAG, "read length: %d", buffer_length);
    buffer = new char[buffer_length];
    memset(buffer, 0, sizeof(char) * buffer_length); 
    err = nvs_get_str(handle, key.c_str(), buffer, &buffer_length);
    if (ESP_OK != err) {
        if (ESP_ERR_NVS_NOT_FOUND != err) {
            ESP_LOGE(NonvolatileStorage::LOG_TAG, 
                     "error code->%d",
                     err);
        }
        delete[] buffer;
        goto DONE;
    }
    ESP_LOGD(LOG_TAG, "read content: %s", buffer);
    result = std::string(buffer);
    delete[] buffer;
DONE:
    nvs_close(handle);
    // 退出临界区
    xSemaphoreGive(mutex);
    return result;
}

bool NonvolatileStorage::WriteString(const std::string &ns, const std::string &key, const char *const value)
{
    bool result = true;
    nvs_handle_t handle;
    esp_err_t err;
    auto key_len = key + "_l";
    NonvolatileStorage::Init();
    if (NVS_KEY_NAME_MAX_SIZE <= key.length()) {
        ESP_LOGE(NonvolatileStorage::LOG_TAG, 
                 "key(%s) >= max key size(%d)",
                 key.c_str(),
                 NVS_KEY_NAME_MAX_SIZE);
        result = false;
        return result;
    }
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    ESP_ERROR_CHECK(nvs_open(ns.c_str(), NVS_READWRITE, &handle));
    err = nvs_erase_key(handle, key_len.c_str());
    if (ESP_OK != err && ESP_ERR_NVS_NOT_FOUND != err) {
        ESP_LOGE(NonvolatileStorage::LOG_TAG, 
                 "error code->%d",
                 err);
        result = false;
        goto DONE;
    }
    err = nvs_erase_key(handle, key.c_str());
    if (ESP_OK != err && ESP_ERR_NVS_NOT_FOUND != err) {
        ESP_LOGE(NonvolatileStorage::LOG_TAG, 
                 "error code->%d",
                 err);
        result = false;
        goto DONE;
    }
    ESP_ERROR_CHECK(nvs_set_u32(handle, key_len.c_str(), (uint32_t)(strlen(value)+1)));
    ESP_ERROR_CHECK(nvs_set_str(handle, key.c_str(), value));
    ESP_ERROR_CHECK(nvs_commit(handle));
    ESP_LOGD(LOG_TAG, "write length: %lu", (uint32_t)(strlen(value)+1));
    ESP_LOGD(LOG_TAG, "write content: %s", value);
DONE:
    nvs_close(handle);
    // 退出临界区
    xSemaphoreGive(mutex);
    return result;
}

bool NonvolatileStorage::WriteString(const std::string &ns, const std::string &key, const std::string &value)
{
    return NonvolatileStorage::WriteString(ns, key, value.c_str());
}

}

}
