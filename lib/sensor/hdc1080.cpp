#include "string.h"

#include "esp_log.h"

#include "system.hpp"

#include "hdc1080.hpp"

namespace cubestone_wang 
{

namespace sensor 
{

using namespace system;

const char *const HDC1080::LOG_TAG = "HDC1080";
uint8_t HDC1080::device_address = 0x40;

HDC1080::HDC1080(I2cMaster* i2c_master)
{
    this->mutex = xSemaphoreCreateMutex();
    this->i2c_master = i2c_master;
    uint8_t data[3] = {
        0x02, 0x00, 0x00
    };
    this->i2c_master->Write(this->device_address, data, 3);
}

float HDC1080::GetTemperature(float offset)
{
    float temperature = 0;
    uint8_t data[2] = {
        0x00, 0x00
    };
    // 设置临界区
    xSemaphoreTake(this->mutex, portMAX_DELAY);
    this->i2c_master->Write(this->device_address, data, 1);
    System::Sleep(25);
    this->i2c_master->Read(this->device_address, data, 2);
    // 退出临界区
    xSemaphoreGive(this->mutex);
    temperature = data[0] * 256 + data[1];
    temperature = temperature * 0.0025177f - 40.0f;  
    temperature += offset;
    return temperature;
}

float HDC1080::GetHumidity(float offset)
{
    float humidity = 0;
    uint8_t data[2] = {
        0x01, 0x00
    };
    // 设置临界区
    xSemaphoreTake(this->mutex, portMAX_DELAY);
    this->i2c_master->Write(this->device_address, data, 1);
    System::Sleep(25);
    this->i2c_master->Read(this->device_address, data, 2);
    // 退出临界区
    xSemaphoreGive(this->mutex);
    humidity = data[0] * 256 + data[1];
    humidity *= 0.001525879f;
    humidity += offset;
    return humidity;
}


}

}
