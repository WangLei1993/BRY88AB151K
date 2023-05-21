#include "string.h"

#include "esp_log.h"

#include "system.hpp"

#include "cm1106.hpp"

namespace cubestone_wang 
{

namespace sensor 
{

using namespace system;

const char *const CM1106::LOG_TAG = "CM1106";

CM1106::CM1106(gpio_num_t rx, gpio_num_t tx, uart_port_t uart_num)
{
    this->mutex = xSemaphoreCreateMutex();
    this->uart_num = uart_num;
    uart_config_t uart_config;
    uart_config.baud_rate = 9600;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.rx_flow_ctrl_thresh = 122;
    uart_config.source_clk = UART_SCLK_DEFAULT;
    ESP_ERROR_CHECK(uart_param_config(this->uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(this->uart_num, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(this->uart_num, 1024, 1024, 0, nullptr, 0));
}

CM1106::~CM1106()
{
    ESP_ERROR_CHECK(uart_driver_delete(this->uart_num));
}

uint16_t CM1106::GetPPM()
{
    uint16_t result_data;
    // 设置临界区
    xSemaphoreTake(this->mutex, portMAX_DELAY);
    uart_flush(this->uart_num);
    uint8_t data[8] = {
        0x11, 0x01, 0x01, 0xed, 0x00, 0x00, 0x00, 0x00
    };
    this->write(data, 4);
    System::Sleep(25);
    this->read(data, 8);
    // 退出临界区
    xSemaphoreGive(this->mutex);
    char buf[8*3];
    for (auto i = 0; i < 8; i++) {
        sprintf(buf+i*3, "%02x%s", data[i], i != (8 -1) ? " " : "");
    }
    ESP_LOGD(CM1106::LOG_TAG, "data: %s", buf);
    uint32_t checksum = 0;
    for (uint32_t i=0; i<7; i++) {
        checksum += data[i];
    }
    checksum = 256-checksum%256;
    if (data[7] != (uint8_t)checksum) {
        ESP_ERROR_CHECK(ESP_ERR_INVALID_CRC);
    } 
    result_data = data[3] * 0x100 + data[4];
    return result_data;
}

void CM1106::Calibrate(uint16_t ppm)
{
    // 设置临界区
    xSemaphoreTake(this->mutex, portMAX_DELAY);
    uart_flush(this->uart_num);
    uint8_t data[6] = {
        0x11, 0x03, 0x03, 0x00, 0x00, 0x00
    };
    data[3] = (uint8_t)(ppm >> 8);
    data[4] = (uint8_t)(ppm & 0xff);
    uint32_t checksum = 0;
    for (uint32_t i=0; i<5; i++) {
        checksum += data[i];
    }
    checksum = 256-checksum%256;
    data[5] = (uint8_t)checksum; 
    this->write(data, 6);
    System::Sleep(25);
    this->read(data, 4);
    xSemaphoreGive(this->mutex);
    if (!(data[0] == 0x16 && data[1] == 0x01 && data[2] == 0x03 && data[3] == 0xe6)) {
        ESP_ERROR_CHECK(ESP_ERR_INVALID_CRC);
    }
    // 退出临界区
    return;
}

void CM1106::write(uint8_t* data, const uint32_t data_length)
{   
    if (uart_write_bytes(this->uart_num, data, data_length) == -1) {
        ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
    };
    return;
}

void CM1106::read(uint8_t* data, const uint32_t data_length)
{   
    time_t start, end, max_wait;
    start = end = System::GetStartupTimestamp();
    max_wait = 5; //最大等待5秒
    uint32_t read_length;
    uint32_t remain_length=data_length;
    while((start + max_wait) > end) 
    {
        read_length = uart_read_bytes(this->uart_num, 
                                      data+(data_length-remain_length), 
                                      remain_length, 
                                      500/portTICK_PERIOD_MS);
        if (read_length == -1) {
            ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
        }
        remain_length -= read_length;
        if (remain_length == 0) {
            return;
        }
        end = System::GetCurrentTimestamp();
    }
    ESP_ERROR_CHECK(ESP_ERR_TIMEOUT);
    return;
}

}

}
