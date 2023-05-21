#ifndef _screen_hpp_
#define _screen_hpp_

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "i2c_master.hpp"
#include "u8g2.hpp"

namespace cubestone_wang 
{

namespace screen
{

using namespace i2c_master;

class Screen
{
    public:
        enum class Status {
            INIT,
            LOADING,
            DISPLAY,
            UNKNOWN
        };
        // 日志标签
        static const char *const LOG_TAG;
        static bool Start(I2cMaster* const i2c_master);
        static void SetContrast(const uint8_t contrast);
        static void SetStatus(const Status status);
        static void SetTemperature(const float temperature);
        static void SetHumidity(const float humidity);
        static void SetCO2(const uint16_t co2);
        static void SetPM25(const uint16_t pm25);
        static void SetPM10(const uint16_t pm10);
        static void SetTVOC(const uint16_t tvoc);
        static void SetCO2eq(const uint16_t co2eq);
        static void SetCH2O(const uint16_t ch2o_ugm3, const uint16_t ch2o_ppb);
    private:
        static SemaphoreHandle_t mutex;
        static bool start_flag;
        static I2cMaster* i2c_master;
        static void run_task(void *);
        static float temperature;   // -40 ~ 125
        static float humidity;      // 0 ~ 100
        static uint16_t co2;        // 0 ~ 5000
        static uint16_t pm25;       //
        static uint16_t pm10;       //
        static uint16_t tvoc;       // 0 ~ 60000
        static uint16_t co2eq;      // 400 ~ 60000
        static uint16_t ch2o_ugm3;  // 0 ~ 6250 (5000 * 1.25)
        static uint16_t ch2o_ppb;   // 0 ~ 5000
        static Status status;
        static Status last_status;
        static uint8_t contrast;
        static uint8_t loading_step;
        static uint8_t display_step;
        static time_t last_update_timestamp;
        static void draw_init(u8g2_t* const u8g2);
        static void draw_loading(u8g2_t* const u8g2);
        static void draw_display(u8g2_t* const u8g2);
};

}

}

#endif // _screen_hpp_
