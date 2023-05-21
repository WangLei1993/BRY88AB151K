#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "system.hpp"

#include "fonts.hpp"
#include "screen.hpp"

namespace cubestone_wang 
{

namespace screen
{

const char *const Screen::LOG_TAG = "SCREEN";
SemaphoreHandle_t Screen::Screen::mutex = xSemaphoreCreateRecursiveMutex();
bool Screen::start_flag = false;
I2cMaster* Screen::i2c_master = nullptr;
float Screen::temperature = 0;
float Screen::humidity = 0;
uint16_t Screen::co2 = 0;
uint16_t Screen::pm25 = 0;
uint16_t Screen::pm10 = 0;
uint16_t Screen::tvoc = 0;
uint16_t Screen::co2eq = 0;
uint16_t Screen::ch2o_ugm3 = 0;
uint16_t Screen::ch2o_ppb = 0;
Screen::Status Screen::status = Screen::Status::INIT;
Screen::Status Screen::last_status = Screen::Status::UNKNOWN;
uint8_t Screen::contrast = 128;
uint8_t Screen::loading_step = 0;
uint8_t Screen::display_step = 0;
time_t Screen::last_update_timestamp = 0;

bool Screen::Start(I2cMaster* const i2c_master)
{
    bool result = true;
    // 设置临界区
    xSemaphoreTakeRecursive(Screen::mutex, portMAX_DELAY);
    // 判断是否已经启动 
    if (true == Screen::start_flag) {
        ESP_LOGI(LOG_TAG, "this has been started");
        result = false;
        goto DONE;
    }
    Screen::i2c_master = i2c_master;
    Screen::start_flag = true;
    if (pdPASS != xTaskCreate(Screen::run_task, "screen", 4096, nullptr, 3, NULL))
    {
        result = false;
    }
DONE:
    // 退出临界区
    xSemaphoreGiveRecursive(Screen::mutex);
    return result;
}

void Screen::SetContrast(const uint8_t contrast)
{
    // 设置临界区
    xSemaphoreTakeRecursive(Screen::mutex, portMAX_DELAY);
    Screen::contrast = contrast;
    // 退出临界区
    xSemaphoreGiveRecursive(Screen::mutex);
}

void Screen::SetStatus(const Status status)
{
    // 设置临界区
    xSemaphoreTakeRecursive(Screen::mutex, portMAX_DELAY);
    Screen::status = status;
    // 退出临界区
    xSemaphoreGiveRecursive(Screen::mutex);
}

void Screen::SetTemperature(const float temperature)
{
    // 设置临界区
    xSemaphoreTakeRecursive(Screen::mutex, portMAX_DELAY);
    Screen::temperature = temperature;
    // 退出临界区
    xSemaphoreGiveRecursive(Screen::mutex);
}

void Screen::SetHumidity(const float humidity)
{
    // 设置临界区
    xSemaphoreTakeRecursive(Screen::mutex, portMAX_DELAY);
    Screen::humidity = humidity;
    // 退出临界区
    xSemaphoreGiveRecursive(Screen::mutex);
}

void Screen::SetCO2(const uint16_t co2)
{
    // 设置临界区
    xSemaphoreTakeRecursive(Screen::mutex, portMAX_DELAY);
    Screen::co2 = co2;
    // 退出临界区
    xSemaphoreGiveRecursive(Screen::mutex);
}

void Screen::SetPM25(const uint16_t pm25)
{
    // 设置临界区
    xSemaphoreTakeRecursive(Screen::mutex, portMAX_DELAY);
    Screen::pm25 = pm25;
    // 退出临界区
    xSemaphoreGiveRecursive(Screen::mutex);
}

void Screen::SetPM10(const uint16_t pm10)
{
    // 设置临界区
    xSemaphoreTakeRecursive(Screen::mutex, portMAX_DELAY);
    Screen::pm10 = pm10;
    // 退出临界区
    xSemaphoreGiveRecursive(Screen::mutex);
}

void Screen::SetTVOC(const uint16_t tvoc)
{
    // 设置临界区
    xSemaphoreTakeRecursive(Screen::mutex, portMAX_DELAY);
    Screen::tvoc = tvoc;
    // 退出临界区
    xSemaphoreGiveRecursive(Screen::mutex);
}

void Screen::SetCO2eq(const uint16_t co2eq)
{
    // 设置临界区
    xSemaphoreTakeRecursive(Screen::mutex, portMAX_DELAY);
    Screen::co2eq = co2eq;
    // 退出临界区
    xSemaphoreGiveRecursive(Screen::mutex);
}

void Screen::SetCH2O(const uint16_t ch2o_ugm3, const uint16_t ch2o_ppb)
{
    // 设置临界区
    xSemaphoreTakeRecursive(Screen::mutex, portMAX_DELAY);
    Screen::ch2o_ugm3 = ch2o_ugm3;
    Screen::ch2o_ppb = ch2o_ppb;
    // 退出临界区
    xSemaphoreGiveRecursive(Screen::mutex);
}

void Screen::run_task(void *)
{
    // 初始化
    auto _u8g2 = u8g2::U8G2(Screen::i2c_master, 
                            0x3c, 
                            u8g2::U8G2::DeviceType::SH1106_I2C_128x64,
                            U8G2_R0);
    auto u8g2_ptr = _u8g2.GetInstance();
    u8g2_SetFont(u8g2_ptr, u8g2_font_t0_14_te);
    u8g2_SetFontPosBottom(u8g2_ptr);
    u8g2_SetFontMode(u8g2_ptr, 1);
    while(true) 
    {   
        // 设置临界区
        xSemaphoreTakeRecursive(Screen::mutex, portMAX_DELAY);
        u8g2_SetContrast(u8g2_ptr, Screen::contrast);
        switch (Screen::status)
        {
            case Screen::Status::INIT:
                Screen::draw_init(u8g2_ptr);
                break;
            case Screen::Status::LOADING:
                Screen::draw_loading(u8g2_ptr);
                break;
            case Screen::Status::DISPLAY:
                Screen::draw_display(u8g2_ptr);
                break;
            default:
                ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
                break;
        }
        Screen::last_status = Screen::status;
        // 退出临界区
        xSemaphoreGiveRecursive(Screen::mutex);
        system::System::Sleep(100);
    }
}

void Screen::draw_init(u8g2_t* const u8g2)
{
    u8g2_ClearBuffer(u8g2);
    u8g2_DrawBox(u8g2, 0, 0, 128, 64);
    u8g2_SendBuffer(u8g2);
}

void Screen::draw_loading(u8g2_t* const u8g2)
{
    u8g2_ClearBuffer(u8g2);
    time_t current_timestamp = system::System::GetStartupTimestamp();
    if (Screen::last_status != Screen::status)
    {
        Screen::loading_step = 0;
        Screen::last_update_timestamp = system::System::GetStartupTimestamp();
    }
    if (current_timestamp-Screen::last_update_timestamp >= 1) {
        Screen::loading_step = (Screen::loading_step + 1) % 4;
        Screen::last_update_timestamp = current_timestamp;
    }
    uint8_t x_pos = 30;
    uint8_t y_pos = 38;
    switch (Screen::loading_step)
    {
        case 0:
            u8g2_DrawUTF8(u8g2, x_pos, y_pos, "Loading");
            break;
        case 1:
            u8g2_DrawUTF8(u8g2, x_pos, y_pos, "Loading.");
            break;
        case 2:
            u8g2_DrawUTF8(u8g2, x_pos, y_pos, "Loading..");
            break;
        case 3:
            u8g2_DrawUTF8(u8g2, x_pos, y_pos, "Loading...");
            break;
        default:
            break;
    }
    u8g2_SendBuffer(u8g2);
}

void Screen::draw_display(u8g2_t* const u8g2)
{
    u8g2_ClearBuffer(u8g2);
    time_t current_timestamp = system::System::GetStartupTimestamp();
    if (Screen::last_status != Screen::status)
    {
        Screen::display_step = 0;
        Screen::last_update_timestamp = system::System::GetStartupTimestamp();
    }
    if (current_timestamp-Screen::last_update_timestamp >= 4) {
        Screen::display_step = (Screen::display_step + 1) % 2;
        Screen::last_update_timestamp = current_timestamp;
    }
    uint8_t x_pos = 8;
    char buf[32];
    switch (Screen::display_step)
    {
        case 0:
            memset(buf, 0, sizeof(char)*32);
            sprintf(buf, "TEMP : %.1f °C", Screen::temperature);
            u8g2_DrawUTF8(u8g2, x_pos, 15, buf);
            memset(buf, 0, sizeof(char)*32);
            sprintf(buf, "RH   : %.1f %%", Screen::humidity);
            u8g2_DrawUTF8(u8g2, x_pos, 31, buf);
            if (Screen::pm25 != 0 || Screen::pm10 != 0) {
                memset(buf, 0, sizeof(char)*32);
                sprintf(buf, "PM2.5: %d ug/m³", Screen::pm25);
                u8g2_DrawUTF8(u8g2, x_pos, 47, buf);
                memset(buf, 0, sizeof(char)*32);
                sprintf(buf, "PM10 : %d ug/m³", Screen::pm10);
                u8g2_DrawUTF8(u8g2, x_pos, 63, buf);
            } else {
                memset(buf, 0, sizeof(char)*32);
                sprintf(buf, "PM2.5: - ug/m³");
                u8g2_DrawUTF8(u8g2, x_pos, 47, buf);
                memset(buf, 0, sizeof(char)*32);
                sprintf(buf, "PM10 : - ug/m³");
                u8g2_DrawUTF8(u8g2, x_pos, 63, buf);
            }
            break;
        case 1:
            memset(buf, 0, sizeof(char)*32);
            sprintf(buf, "CO₂  : %d ppm", Screen::co2);
            u8g2_DrawUTF8(u8g2, x_pos, 15, buf);
            memset(buf, 0, sizeof(char)*32);
            sprintf(buf, "CH₂O : %d ug/m³", Screen::ch2o_ugm3);
            u8g2_DrawUTF8(u8g2, x_pos, 47, buf);
            if (Screen::tvoc != 0 || Screen::co2eq != 400) {
                memset(buf, 0, sizeof(char)*32);
                sprintf(buf, "TVOC : %d ppb", Screen::tvoc);
                u8g2_DrawUTF8(u8g2, x_pos, 31, buf);
                memset(buf, 0, sizeof(char)*32);
                sprintf(buf, "CO₂eq: %d ug/m³", Screen::co2eq);
                u8g2_DrawUTF8(u8g2, x_pos, 63, buf);
            } else {
                memset(buf, 0, sizeof(char)*32);
                sprintf(buf, "TVOC : - ppb");
                u8g2_DrawUTF8(u8g2, x_pos, 31, buf);
                memset(buf, 0, sizeof(char)*32);
                sprintf(buf, "CO₂eq: - ug/m³");
                u8g2_DrawUTF8(u8g2, x_pos, 63, buf);
            }
            break;
        default:
            break;
    }
    // u8g2_DrawLine(u8g2, 0, 0, 127, 0);
    // u8g2_DrawLine(u8g2, 127, 0, 127, 63);
    // u8g2_DrawLine(u8g2, 127, 63, 0, 63);
    // u8g2_DrawLine(u8g2, 0, 63, 0, 0);
    u8g2_SendBuffer(u8g2);
}

}

}
