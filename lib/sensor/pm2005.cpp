#include "string.h"

#include "esp_log.h"

#include "pm2005.hpp"

namespace cubestone_wang 
{

namespace sensor 
{

const char *const PM2005::LOG_TAG = "PM2005";
uint8_t PM2005::device_address = 0x28;


PM2005::PM2005(I2cMaster* i2c_master)
{
    this->mutex = xSemaphoreCreateMutex();
    this->i2c_master = i2c_master;
    uint8_t data[7] = {
        0x16, 0x07, 0x05, 0x00, 0x24, 0x00, 0x00
    };
    data[6] = data[0] ^ data[1] ^ data[2] ^ data[3] ^ data[4] ^ data[5];
    this->i2c_master->Write(this->device_address, data, 7);
}

PM2005::Data PM2005::GetData()
{
    uint8_t data[22];
    Data result_data;
    result_data.PM10 = 0;
    result_data.PM25 = 0;
    // 设置临界区
    xSemaphoreTake(this->mutex, portMAX_DELAY);
    this->i2c_master->Read(this->device_address, data, 22);
    // 退出临界区
    xSemaphoreGive(this->mutex);
    char buf[22*3];
    for (auto i = 0; i < 22; i++) {
        sprintf(buf+i*3, "%02x%s", data[i], i != (22 -1) ? " " : "");
    }
    ESP_LOGD(PM2005::LOG_TAG, "data: %s", buf);
    if (data[2] == 0x80) {
        result_data.PM25 = data[5] * 0x100 + data[6];
        result_data.PM10 = data[7] * 0x100 + data[8];
    }
    return result_data;
}

}

}
