#include "esp_log.h"

#include "monochrome_led.hpp"

namespace cubestone_wang 
{

namespace monochrome_led
{

const char *const MonochromeLED::LOG_TAG = "MONOCHROME_LED";

MonochromeLED::MonochromeLED(const gpio_num_t pin, const bool reverse_on_off)
{
    this->pin = pin;
    if (reverse_on_off) {
        this->GPIO_LEVEL_ON = 0;
        this->GPIO_LEVEL_OFF = 1;
    } else {
        this->GPIO_LEVEL_ON = 1;
        this->GPIO_LEVEL_OFF = 0;
    }
    ESP_ERROR_CHECK(gpio_reset_pin(this->pin));
    ESP_ERROR_CHECK(gpio_set_intr_type(this->pin, GPIO_INTR_DISABLE));
    ESP_ERROR_CHECK(gpio_set_direction(this->pin, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_level(this->pin, this->GPIO_LEVEL_OFF));
    this->mutex = xSemaphoreCreateMutex();
}

void MonochromeLED::SetOn()
{
    xSemaphoreTake(this->mutex, portMAX_DELAY);
    ESP_ERROR_CHECK(gpio_set_level(this->pin, this->GPIO_LEVEL_ON));
    xSemaphoreGive(this->mutex);
}

void MonochromeLED::SetOff()
{
    xSemaphoreTake(this->mutex, portMAX_DELAY);
    ESP_ERROR_CHECK(gpio_set_level(this->pin, this->GPIO_LEVEL_OFF));
    xSemaphoreGive(this->mutex);
}

}

}
