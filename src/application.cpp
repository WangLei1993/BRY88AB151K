#include <vector>

#include "cJSON.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "lwip/netdb.h"

#include "button_manager.hpp"
#include "config_manager.hpp"
#include "influxdb_config.hpp"
#include "mdns_config.hpp"
#include "mdns.hpp"
#include "monochrome_led.hpp"
#include "monochrome_led_manager.hpp"
#include "ntp_config.hpp"
#include "ntp.hpp"
#include "scheduled_restart_config.hpp"
#include "scheduled_restart.hpp"
#include "sgp30_config.hpp"
#include "system.hpp"
#include "wifi_config.hpp"
#include "wifi.hpp"

#include "screen/screen.hpp"

#include "application.hpp"

namespace cubestone_wang 
{

namespace application
{

const char * const Application::LOG_TAG = "APPLICATION";

bool Application::start_flag = false;

SemaphoreHandle_t Application::mutex = xSemaphoreCreateMutex();

std::string Application::wifi_monochrome_led_name = "wifi";
std::string Application::pm25_monochrome_led_name = "pm25";
std::string Application::co2_monochrome_led_name = "co2";
std::string Application::tvoc_monochrome_led_name = "tvoc";
std::string Application::inner_monochrome_led_name = "inner";
std::string Application::sgp30_config_name = "sgp30";
std::string Application::scheduled_restart_config_name = "restart";
std::string Application::influxdb_config_name = "influxdb";
std::string Application::ntp_config_name = "ntp";
std::string Application::mdns_config_name = "mdns";
std::string Application::wifi_config_name = "wifi";

i2c_master::I2cMaster *Application::i2c_master_0 = nullptr;
i2c_master::I2cMaster *Application::i2c_master_1 = nullptr;
sensor::CM1106 *Application::cm1106 = nullptr;
sensor::HDC1080 *Application::hdc1080 = nullptr;
sensor::PM2005 *Application::pm2005 = nullptr;
sensor::SGP30 *Application::sgp30 = nullptr;
sensor::ZE08_CH2O *Application::ze08_ch2o = nullptr;
influxdb::Influxdb *Application::influxdb = nullptr;
QueueHandle_t Application::influxdb_queue = xQueueCreate(100, sizeof(influxdb::Point *));

bool Application::init()
{
    if (false == Application::init_log()) {
        return false;
    }
    if (false == Application::init_monochrome_led()) {
        return false;
    }
    if (false == Application::init_button()) {
        return false;
    }
    if (false == Application::init_config()) {
        return false;
    }
    if (false == Application::init_i2c()) {
        return false;
    }
    if (false == Application::init_uart()) {
        return false;
    }
    if (false == Application::init_influxdb()) {
        return false;
    }
    return true;
}

bool Application::init_log()
{
    esp_log_level_set(LOG_TAG, ESP_LOG_DEBUG);
    esp_log_level_set("wifi", ESP_LOG_ERROR);
    esp_log_level_set(config::ConfigManager::LOG_TAG, ESP_LOG_DEBUG);
    // esp_log_level_set(sensor::PM2005::LOG_TAG, ESP_LOG_DEBUG);
    // esp_log_level_set(sensor::CM1106::LOG_TAG, ESP_LOG_DEBUG);
    // esp_log_level_set(sensor::SGP30::LOG_TAG, ESP_LOG_DEBUG);
    // esp_log_level_set(sensor::ZE08_CH2O::LOG_TAG, ESP_LOG_DEBUG);
    return true;
}

bool Application::init_button()
{
    esp_log_level_set(button::ButtonManager::LOG_TAG, ESP_LOG_WARN);
    auto button_name = std::string("button");
    button::ButtonManager::Add(button_name, GPIO_NUM_34);
    // 设置回调函数
    button::ButtonManager::SetClickCallbackFunction(button_name, [](void *_monochrome_led_name) {
        auto func = [](void *_monochrome_led_name)
        {
            auto monochrome_led_name = (std::string *)_monochrome_led_name;
            monochrome_led::MonochromeLEDManager::SetOn(*monochrome_led_name);
            system::System::Sleep(200);
            monochrome_led::MonochromeLEDManager::SetOff(*monochrome_led_name);
            vTaskDelete(NULL);
        };
        xTaskCreate(func, "tmp", 1024, _monochrome_led_name, 2, NULL);
        auto task_info_summary = system::System::GetCurrentTaskInfoSummary();
        uint32_t stats_as_percentage;
        auto task_state_map = [](eTaskState state) {
            switch (state)
            {
                case eTaskState::eRunning:
                    return 'X';
                case eTaskState::eReady:
                    return 'R';
                case eTaskState::eBlocked:
                    return 'B';
                case eTaskState::eSuspended:
                    return 'S';
                case eTaskState::eDeleted:
                    return 'D';
                default:
                    return 'E';
            }
        };
        task_info_summary.TotalRuntime /= 100UL;
        if (task_info_summary.TotalRuntime == 0) {
            ESP_LOGE(LOG_TAG, "total_runtime is 0");
        } else {
            ESP_LOGI(LOG_TAG, "core    id              name  pri.  state   stack     runtime   runtime(%%)");
            for (auto task_info : task_info_summary.TaskInfoList) {
                stats_as_percentage = task_info.Runtime / task_info_summary.TotalRuntime;
                if (stats_as_percentage > 0) {
                    ESP_LOGI(LOG_TAG, "%4ld%6lu  %16s  %4lu  %5c  %6lu  %10lu  %10lu%%",
                                task_info.CoreID,
                                task_info.Id, 
                                task_info.Name.c_str(),
                                task_info.Priority,
                                task_state_map(task_info.State),
                                task_info.StackHighWaterMark,
                                task_info.Runtime,
                                stats_as_percentage);
                } else {
                    ESP_LOGI(LOG_TAG, "%4ld%6lu  %16s  %4lu  %5c  %6lu  %10lu          <1%%",
                                task_info.CoreID,
                                task_info.Id, 
                                task_info.Name.c_str(),
                                task_info.Priority,
                                task_state_map(task_info.State),
                                task_info.StackHighWaterMark,
                                task_info.Runtime);
                }
            }
        }
        uint32_t free_heap_size = system::System::GetCurrentFreeHeapSize();
        uint32_t min_free_heap_size = system::System::GetCurrentMinimumFreeHeapSize();
        ESP_LOGI(LOG_TAG, "current free heap size: %luB, %.2fKiB", 
                    free_heap_size, free_heap_size/1024.0);
        ESP_LOGI(LOG_TAG, "min free heap size: %luB, %.2fKiB", 
                    min_free_heap_size, min_free_heap_size/1024.0);
        ESP_LOGI(Application::LOG_TAG, "uptime: %s", system::System::GetStartupTimeString().c_str());
    }, &Application::wifi_monochrome_led_name);
    button::ButtonManager::SetDoubleClickCallbackFunction(button_name, [](void *_monochrome_led_name) {
        auto func = [](void *_monochrome_led_name)
        {
            auto monochrome_led_name = (std::string *)_monochrome_led_name;
            for(auto i=0; i<2; i++) {
                monochrome_led::MonochromeLEDManager::SetOn(*monochrome_led_name);
                system::System::Sleep(200);
                monochrome_led::MonochromeLEDManager::SetOff(*monochrome_led_name);
                system::System::Sleep(200);
            }
            vTaskDelete(NULL);
        };
        xTaskCreate(func, "tmp", 1024, _monochrome_led_name, 2, NULL);
        Application::cm1106->Calibrate();
    }, &Application::wifi_monochrome_led_name);
    button::ButtonManager::SetMultiClickCallbackFunction(button_name, [](void *_monochrome_led_name) {
        auto func = [](void *_monochrome_led_name)
        {
            auto monochrome_led_name = (std::string *)_monochrome_led_name;
            for(auto i=0; i<3; i++) {
                monochrome_led::MonochromeLEDManager::SetOn(*monochrome_led_name);
                system::System::Sleep(200);
                monochrome_led::MonochromeLEDManager::SetOff(*monochrome_led_name);
                system::System::Sleep(200);
            }
            vTaskDelete(NULL);
        };
        xTaskCreate(func, "tmp", 1024, _monochrome_led_name, 2, NULL);
        if (!config::ConfigManager::Reset()) {
            ESP_LOGE(Application::LOG_TAG, "reset failed");
            return;
        }
        auto wifi_config = (wifi::Config *)config::ConfigManager::Get(Application::wifi_config_name);
        wifi_config->STA.SSID = "{your_wifi_name}";
        wifi_config->STA.Password = "{your_wifi_password}";
        auto influxdb_config = (influxdb::Config *)config::ConfigManager::Get(Application::influxdb_config_name);
        influxdb_config->Host = "influxdb.local";
        influxdb_config->Port = 8086;
        influxdb_config->Token = "{your_influxdb_token}";
        influxdb_config->Org = "default";
        influxdb_config->Bucket = "sensor";
        influxdb_config->Timeout = 5;
        if (!config::ConfigManager::Save())
        {
            ESP_LOGE(Application::LOG_TAG, "save after reset failed");
        }
        system::System::Restart("restart after reset config");
    }, &Application::wifi_monochrome_led_name);
    button::ButtonManager::SetLongPressStartCallbackFunction(button_name, [](void *_monochrome_led_name) {
        auto monochrome_led_name = (std::string *)_monochrome_led_name;
        monochrome_led::MonochromeLEDManager::SetOn(*monochrome_led_name);
    }, &Application::wifi_monochrome_led_name);
    button::ButtonManager::SetLongPressStopCallbackFunction(button_name, [](void *_monochrome_led_name) {
        auto monochrome_led_name = (std::string *)_monochrome_led_name;
        system::System::Restart("human restart", 5);
        monochrome_led::MonochromeLEDManager::SetOff(*monochrome_led_name);
    }, &Application::wifi_monochrome_led_name);
    return true;
}

bool Application::init_config()
{
    config::ConfigManager::Add(Application::wifi_config_name, new wifi::Config());
    config::ConfigManager::Add(Application::ntp_config_name, new ntp::Config());
    config::ConfigManager::Add(Application::mdns_config_name, new mdns::Config());
    config::ConfigManager::Add(Application::scheduled_restart_config_name, new scheduled_restart::Config());
    config::ConfigManager::Add(Application::sgp30_config_name, new sensor::SGP30Config());
    config::ConfigManager::Add(Application::influxdb_config_name, new influxdb::Config());
    return config::ConfigManager::Load();
}

bool Application::init_monochrome_led()
{
    monochrome_led::MonochromeLEDManager::Add(Application::wifi_monochrome_led_name, GPIO_NUM_25);
    monochrome_led::MonochromeLEDManager::Add(Application::pm25_monochrome_led_name, GPIO_NUM_33);
    monochrome_led::MonochromeLEDManager::Add(Application::co2_monochrome_led_name, GPIO_NUM_4);
    monochrome_led::MonochromeLEDManager::Add(Application::tvoc_monochrome_led_name, GPIO_NUM_32);
    monochrome_led::MonochromeLEDManager::Add(Application::inner_monochrome_led_name, GPIO_NUM_26);
    return true;
}

bool Application::init_i2c()
{

    Application::i2c_master_0 = new i2c_master::I2cMaster(GPIO_NUM_19, GPIO_NUM_21, 200000, 0);
    Application::i2c_master_1 = new i2c_master::I2cMaster(GPIO_NUM_23, GPIO_NUM_22, 200000, 1);
    Application::hdc1080 = new sensor::HDC1080(Application::i2c_master_0);
    Application::sgp30 = new sensor::SGP30(Application::i2c_master_0);
    Application::pm2005 = new sensor::PM2005(Application::i2c_master_1);
    sensor::SGP30Config *spg30_config = (sensor::SGP30Config *)config::ConfigManager::Get(Application::sgp30_config_name);
    if (spg30_config->CO2eq != spg30_config->Default_CO2eq && spg30_config->TVOC != spg30_config->Default_TVOC) {
        sensor::SGP30::Baseline baseline;
        baseline.CO2eq = spg30_config->CO2eq;
        baseline.TVOC = spg30_config->TVOC;
        Application::sgp30->SetBaseline(baseline);
    }
    screen::Screen::Start(Application::i2c_master_1);
    return true;
}

bool Application::init_uart()
{
    Application::cm1106 = new sensor::CM1106(GPIO_NUM_16, GPIO_NUM_17, 2);
    Application::ze08_ch2o = new sensor::ZE08_CH2O(GPIO_NUM_5, GPIO_NUM_18, 1);
    return true;
}

bool Application::init_influxdb()
{
    influxdb::Config *influxdb_config = (influxdb::Config*)config::ConfigManager::Get(Application::influxdb_config_name);
    Application::influxdb = new influxdb::Influxdb(influxdb_config->Host,
                                                   influxdb_config->Port,
                                                   influxdb_config->Token,
                                                   influxdb_config->Org,
                                                   influxdb_config->Bucket,
                                                   influxdb_config->Timeout);
    auto func = [](void *args)
    {
        while (true) {
            influxdb::Point *point;
            if (pdTRUE != xQueueReceive(Application::influxdb_queue, (void *)&point, portMAX_DELAY)) {
                continue;
            }
            if (!Application::influxdb->WritePoint(*point)) {
                ESP_LOGE(LOG_TAG, "write to influxdb failed");
            } else {
                ESP_LOGI(LOG_TAG, "write to influxdb success");
            }
            delete point;
            point = nullptr;
        }
    };
    xTaskCreate(func, "influxdb", 4096, nullptr, 2, NULL);
    return true;
}

void Application::reboot_for_failed_start(std::string reason)
{   
    start_flag = false;
    system::System::Restart(reason.c_str(), 10);
}

void Application::shutdown_handler()
{
    start_flag = false;
    mdns::MDNS::Stop();
    ESP_LOGI(Application::LOG_TAG, "mdns has been stopped");
    ntp::NTP::Stop();
    ESP_LOGI(Application::LOG_TAG, "ntp has been stopped");
    wifi::WiFi::Stop();
    ESP_LOGI(Application::LOG_TAG, "wifi has been stopped");
    fflush(stdout);
    return;
}

void Application::Start()
{   
    // 判断是否已经启动
    // 设置临界区
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (start_flag == true) {
        ESP_LOGI(LOG_TAG, "this has been started");
        // 退出临界区
        xSemaphoreGive(mutex);
        return;
    }
    start_flag = true;
    // 退出临界区
    xSemaphoreGive(mutex);

    // 初始化组件
    if (!Application::init()) {
        ESP_LOGE(Application::LOG_TAG, "init error, run stopped");
        return;
    }

    // 输出基础信息
    system::System::LogHardwareInfo();
    system::System::LogSoftwareInfo();

    button::ButtonManager::Start();
    monochrome_led::MonochromeLEDManager::Start();
    monochrome_led::MonochromeLEDManager::SetBlink(Application::inner_monochrome_led_name, 500, 500);
    
    monochrome_led::MonochromeLEDManager::SetBlink(Application::wifi_monochrome_led_name, 250, 250);
    
    screen::Screen::SetStatus(screen::Screen::Status::LOADING);

    auto wifi_config = (wifi::Config *)config::ConfigManager::Get(Application::wifi_config_name);
    if(!wifi::WiFi::Start(WIFI_MODE_STA, 
                          wifi_config->STA.SSID, 
                          wifi_config->STA.Password, 
                          wifi_config->STA.Hostname)) {
        reboot_for_failed_start("wifi start failed");
        return;
    }
    ESP_LOGI(LOG_TAG, "wifi start success");

    auto ntp_config = (ntp::Config *)config::ConfigManager::Get(Application::ntp_config_name);
    if(!ntp::NTP::Start(ntp_config->ServerNameList)) {
        ESP_LOGE(LOG_TAG, "ntp start failed");
        reboot_for_failed_start("ntp start failed");
        return;
    } 
    ESP_LOGI(LOG_TAG, "ntp start success");
    ESP_LOGI(LOG_TAG, "the current date/time is: %s", system::System::GetCurrentTimeStandardString().c_str());
    
    auto scheduled_restart = (scheduled_restart::Config *)config::ConfigManager::Get(Application::scheduled_restart_config_name);
    if(!scheduled_restart::ScheduledRestart::Start(scheduled_restart->DayType, 
                                                   scheduled_restart->Hour,
                                                   scheduled_restart->Minute)) {
        reboot_for_failed_start("scheduled restart start failed");
        return;
    } 
    ESP_LOGI(LOG_TAG, "scheduled restart start success");

    auto mdns_config = (mdns::Config *)config::ConfigManager::Get(Application::mdns_config_name);
    if(!mdns::MDNS::Start(mdns_config->Hostname, 
                          mdns_config->InstanceName,
                          mdns_config->services)) {
        std::string message = "mdns start failed";
        reboot_for_failed_start(message);
        return;
    } 
    ESP_LOGI(LOG_TAG, "mdns start success");
   
    // 注册重启时的回调函数
    ESP_ERROR_CHECK(esp_register_shutdown_handler(shutdown_handler));

    // 注册看门狗
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    
    screen::Screen::SetStatus(screen::Screen::Status::DISPLAY);
    
    auto last_startup_timestamp = system::System::GetStartupTimestamp();
    std::string measurement = mdns_config->Hostname;
    // 主循环
    while(1) {
        monochrome_led::MonochromeLEDManager::SetBlink(Application::wifi_monochrome_led_name, 1000, 2000);
        uint32_t count = 0;
        while (count < 3)
        {   
            auto temperature = Application::hdc1080->GetTemperature(-5);
            auto humidity = Application::hdc1080->GetHumidity(12.5);
            auto pm2005_data = Application::pm2005->GetData();
            auto cm1106_data = Application::cm1106->GetPPM();
            auto sgp_data = Application::sgp30->GetData(temperature, humidity);
            auto sgp_baseline = Application::sgp30->GetBaseline();
            auto current_startup_timestamp = system::System::GetStartupTimestamp();
            auto ze08_ch2o_data = Application::ze08_ch2o->GetData();
            
            if ((current_startup_timestamp-last_startup_timestamp) >= 1800) {
                sensor::SGP30Config *config = (sensor::SGP30Config *)config::ConfigManager::Get(Application::sgp30_config_name);
                config->CO2eq = sgp_baseline.CO2eq;
                config->TVOC = sgp_baseline.TVOC;
                if (config::ConfigManager::Save(Application::sgp30_config_name)) {
                    ESP_LOGI(LOG_TAG, "sgp30 baseline save success");
                } else {
                    ESP_LOGE(LOG_TAG, "sgp30 baseline save failed");
                }
                last_startup_timestamp = current_startup_timestamp;
            }

            ESP_LOGI(LOG_TAG, "Temperature: %.1f℃", temperature);
            ESP_LOGI(LOG_TAG, "Humidity: %.1f%%", humidity);
            ESP_LOGI(LOG_TAG, "PM2.5: %uμg/m³, PM10: %uμg/m³", pm2005_data.PM25, pm2005_data.PM10);
            ESP_LOGI(LOG_TAG, "CO₂: %uppm", cm1106_data);
            ESP_LOGI(LOG_TAG, "TVOC: %uppb, CO₂eq: %uppm", sgp_data.TVOC, sgp_data.CO2eq);
            ESP_LOGI(LOG_TAG, "CH₂O: %uμg/m³, %uppb", ze08_ch2o_data.CH2O_UGM3, ze08_ch2o_data.CH2O_PPB);
            
            screen::Screen::SetTemperature(temperature);
            screen::Screen::SetHumidity(humidity);
            screen::Screen::SetPM25(pm2005_data.PM25);
            screen::Screen::SetPM10(pm2005_data.PM10);
            screen::Screen::SetCO2(cm1106_data);
            screen::Screen::SetTVOC(sgp_data.TVOC);
            screen::Screen::SetCO2eq(sgp_data.CO2eq);
            screen::Screen::SetCH2O(ze08_ch2o_data.CH2O_UGM3, ze08_ch2o_data.CH2O_PPB);

            if (pm2005_data.PM25 > 75) {
                monochrome_led::MonochromeLEDManager::SetOn(Application::pm25_monochrome_led_name);
            } else {
                monochrome_led::MonochromeLEDManager::SetOff(Application::pm25_monochrome_led_name);
            }
            if (cm1106_data > 1000) {
                monochrome_led::MonochromeLEDManager::SetOn(Application::co2_monochrome_led_name);
            } else {
                monochrome_led::MonochromeLEDManager::SetOff(Application::co2_monochrome_led_name);
            }
            if (sgp_data.TVOC > 500 || ze08_ch2o_data.CH2O_UGM3 > 80) {
                monochrome_led::MonochromeLEDManager::SetOn(Application::tvoc_monochrome_led_name);
            } else {
                monochrome_led::MonochromeLEDManager::SetOff(Application::tvoc_monochrome_led_name);
            }

            auto point = new influxdb::Point(measurement);
            point->AddField("temperature", temperature);
            point->AddField("humidity", humidity);
            point->AddField("co2", (long long)cm1106_data);
            if (pm2005_data.PM25 != 0 || pm2005_data.PM10 !=0) {
                point->AddField("pm25", (long long)pm2005_data.PM25);
                point->AddField("pm10", (long long)pm2005_data.PM10);
            }
            if (sgp_data.TVOC != 0 || sgp_data.CO2eq != 400) {
                point->AddField("tvoc", (long long)sgp_data.TVOC);
                point->AddField("co2eq", (long long)sgp_data.CO2eq);
            }
            
            point->AddField("ch2o_ugm3", (long long)ze08_ch2o_data.CH2O_UGM3);
            point->AddField("ch2o_ppb", (long long)ze08_ch2o_data.CH2O_PPB);
            point->SetTimestamp(system::System::GetCurrentTimestamp());
            xQueueSend(Application::influxdb_queue, (void *)&point, portMAX_DELAY);

            count += 1;
            esp_task_wdt_reset();
            system::System::Sleep(2500);
            esp_task_wdt_reset();
            system::System::Sleep(2500);
            esp_task_wdt_reset();
        }
    }
}

}

}
