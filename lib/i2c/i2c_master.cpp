#include "string.h"

#include "esp_log.h"

#include "i2c_master.hpp"

namespace cubestone_wang 
{

namespace i2c_master 
{

const char *const I2cMaster::LOG_TAG = "I2cMaster";

I2cMaster::I2cMaster(gpio_num_t sda, 
                     gpio_num_t scl, 
                     uint32_t clk_speed, 
                     i2c_port_t i2c_num)
{
    mutex = xSemaphoreCreateMutex();
    this->i2c_num = i2c_num;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = scl;       
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = clk_speed;                    
    conf.clk_flags = 0;
    ESP_ERROR_CHECK(i2c_param_config(this->i2c_num, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(this->i2c_num, conf.mode, 0, 0, 0));
}

I2cMaster::~I2cMaster()
{
    ESP_ERROR_CHECK(i2c_driver_delete(this->i2c_num));
}

void I2cMaster::Write(const uint8_t device_address, 
                      const uint8_t* const write_buffer, 
                      const size_t write_size)
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    ESP_ERROR_CHECK(i2c_master_write_to_device(this->i2c_num, 
                                               device_address, 
                                               write_buffer, 
                                               write_size, 
                                               1000/portTICK_PERIOD_MS));
    // 退出临界区
    xSemaphoreGive(mutex);
    return;
}

void I2cMaster::Read(const uint8_t device_address, 
                     uint8_t* read_buffer, 
                     const size_t read_size)
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    ESP_ERROR_CHECK(i2c_master_read_from_device(this->i2c_num, 
                                                device_address,
                                                read_buffer, 
                                                read_size, 
                                                1000/portTICK_PERIOD_MS));
    // 退出临界区
    xSemaphoreGive(mutex);
    return;
}

void I2cMaster::ReadAfterWrite(const uint8_t device_address, 
                               const uint8_t* const write_buffer, 
                               const size_t write_size, 
                               uint8_t* read_buffer, 
                               const size_t read_size)
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    ESP_ERROR_CHECK(i2c_master_write_read_device(this->i2c_num, 
                                                device_address,
                                                write_buffer,
                                                write_size,
                                                read_buffer, 
                                                read_size, 
                                                1000/portTICK_PERIOD_MS));
    // 退出临界区
    xSemaphoreGive(mutex);
    return;
}

void I2cMaster::SearchAddress()
{
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    esp_err_t err;
    for (uint8_t address = 0x08; address <= 0x77; address++) {
        i2c_cmd_handle_t handle = i2c_cmd_link_create();
        err = i2c_master_start(handle);
        if (err != ESP_OK) {
            goto end;
        }
        err = i2c_master_write_byte(handle, address << 1 | I2C_MASTER_WRITE, true);
        if (err != ESP_OK) {
            goto end;
        }
        i2c_master_stop(handle);
        err = i2c_master_cmd_begin(i2c_num, handle, 1000/portTICK_PERIOD_MS);
    end:
        i2c_cmd_link_delete(handle);
        if (err == ESP_OK) {
            ESP_LOGD(LOG_TAG, "address 0x%02x is found", address);
        }
    }
    // 退出临界区
    xSemaphoreGive(mutex);
    return;
}

}

}
