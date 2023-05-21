#include "string.h"
#include "cmath"

#include "esp_log.h"

#include "system.hpp"

#include "sgp30.hpp"

namespace cubestone_wang 
{

namespace sensor 
{

using namespace system;

const char *const SGP30::LOG_TAG = "SGP30";
uint8_t SGP30::device_address = 0x58;

SGP30::SGP30(I2cMaster* i2c_master)
{
    this->mutex = xSemaphoreCreateMutex();
    this->i2c_master = i2c_master;
    uint8_t data[2] = {
        0x20,
        0x03
    };
    this->i2c_master->Write(this->device_address, data, 2);
}

SGP30::Data SGP30::GetData(float temperature, float humidity)
{
    Data result_data;
    float absolute_humidity =
        216.7f * (((humidity / 100) * 6.112f * std::exp((17.62f * temperature) / (243.12f + temperature))) / (273.15f + temperature));
    uint8_t data[6] = {
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    // 设置临界区
    xSemaphoreTake(this->mutex, portMAX_DELAY);
    data[0] = 0x20;
    data[1] = 0x61;
    data[2] = uint8_t(std::floor(absolute_humidity));
    data[3] = uint8_t(std::floor((absolute_humidity - std::floor(absolute_humidity)) * 256));
    data[4] = crc(data[2], data[3]);
    this->i2c_master->Write(this->device_address, data, 5);
    System::Sleep(50);
    data[0] = 0x20;
    data[1] = 0x08;
    this->i2c_master->Write(this->device_address, data, 2);
    System::Sleep(25);
    this->i2c_master->Read(this->device_address, data, 6);
    // 退出临界区
    xSemaphoreGive(this->mutex);
    char buf[6*3];
    for (auto i = 0; i < 6; i++) {
        sprintf(buf+i*3, "%02x%s", data[i], i != (6 -1) ? " " : "");
    }
    ESP_LOGD(SGP30::LOG_TAG, "data: %s", buf);
    if (crc(data[0], data[1]) != data[2] || crc(data[3], data[4]) != data[5]) {
        ESP_ERROR_CHECK(ESP_ERR_INVALID_CRC);
    }
    result_data.CO2eq = data[0] * 0x100 + data[1]; 
    result_data.TVOC = data[3] * 0x100 + data[4]; 
    return result_data;
}

SGP30::Baseline SGP30::GetBaseline()
{   
    Baseline baseline;
    uint8_t data[6] = {
       0x20, 0x15, 0x00, 0x00, 0x00, 0x00
    };
    // 设置临界区
    xSemaphoreTake(this->mutex, portMAX_DELAY);
    this->i2c_master->Write(this->device_address, data, 2);
    System::Sleep(25);
    this->i2c_master->Read(this->device_address, data, 6);
    // 退出临界区
    xSemaphoreGive(this->mutex);
    char buf[6*3];
    for (auto i = 0; i < 6; i++) {
        sprintf(buf+i*3, "%02x%s", data[i], i != (6 -1) ? " " : "");
    }
    ESP_LOGD(SGP30::LOG_TAG, "data: %s", buf);
    if (crc(data[0], data[1]) != data[2] || crc(data[3], data[4]) != data[5]) {
        ESP_ERROR_CHECK(ESP_ERR_INVALID_CRC);
    }
    baseline.CO2eq = data[0] * 0x100 + data[1]; 
    baseline.TVOC = data[3] * 0x100 + data[4]; 
    return baseline;
}

void SGP30::SetBaseline(Baseline baseline)
{
    uint8_t data[8] = {
       0x20, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    data[2] = uint8_t(baseline.CO2eq >> 8 & 0xff);
    data[3] = uint8_t(baseline.CO2eq & 0xff);
    data[4] = crc(data[2], data[3]);
    data[5] = uint8_t(baseline.TVOC >> 8 & 0xff);
    data[6] = uint8_t(baseline.TVOC & 0xff);
    data[7] = crc(data[2], data[3]);
    // 设置临界区
    xSemaphoreTake(this->mutex, portMAX_DELAY);
    this->i2c_master->Write(this->device_address, data, 2);
    // 退出临界区
    xSemaphoreGive(this->mutex);
    return;
}

uint8_t SGP30::crc(uint8_t data1, uint8_t data2)
{
    uint8_t bit;
    uint8_t crc = 0xFF;
    crc ^= data1;
    for (bit = 8; bit > 0; --bit) {
        if (crc & 0x80) {
        crc = (crc << 1) ^ 0x31u;
        } else {
        crc = (crc << 1);
        }
    }
    crc ^= data2;
    for (bit = 8; bit > 0; --bit) {
        if (crc & 0x80) {
        crc = (crc << 1) ^ 0x31u;
        } else {
        crc = (crc << 1);
        }
    }
    return crc; 
}

}

}
