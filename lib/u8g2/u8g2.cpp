#include <functional>
#include "esp_log.h"
#include "system.hpp"
#include "u8g2.hpp"

namespace cubestone_wang 
{

namespace u8g2
{

const char *const U8G2::LOG_TAG = "U8G2";

uint8_t U8G2::u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch (msg)
    {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:	// called once during init phase of u8g2/u8x8
            break;							// can be used to setup pins
        case U8X8_MSG_DELAY_NANO:			// delay arg_int * 1 nano second
            break;    
        case U8X8_MSG_DELAY_100NANO:		// delay arg_int * 100 nano seconds
            break;
        case U8X8_MSG_DELAY_10MICRO:		// delay arg_int * 10 micro seconds
            break;
        case U8X8_MSG_DELAY_MILLI:			// delay arg_int * 1 milli second
            system::System::Sleep(arg_int);
            break;
        case U8X8_MSG_DELAY_I2C:				// arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz
            break;							// arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
        case U8X8_MSG_GPIO_D0:				// D0 or SPI clock pin: Output level in arg_int
        // case U8X8_MSG_GPIO_SPI_CLOCK:
            break;
        case U8X8_MSG_GPIO_D1:				// D1 or SPI data pin: Output level in arg_int
        //case U8X8_MSG_GPIO_SPI_DATA:
            break;
        case U8X8_MSG_GPIO_D2:				// D2 pin: Output level in arg_int
            break;
        case U8X8_MSG_GPIO_D3:				// D3 pin: Output level in arg_int
            break;
        case U8X8_MSG_GPIO_D4:				// D4 pin: Output level in arg_int
            break;
        case U8X8_MSG_GPIO_D5:				// D5 pin: Output level in arg_int
            break;
        case U8X8_MSG_GPIO_D6:				// D6 pin: Output level in arg_int
            break;
        case U8X8_MSG_GPIO_D7:				// D7 pin: Output level in arg_int
            break;
        case U8X8_MSG_GPIO_E:				// E/WR pin: Output level in arg_int
            break;
        case U8X8_MSG_GPIO_CS:				// CS (chip select) pin: Output level in arg_int
            break;
        case U8X8_MSG_GPIO_DC:				// DC (data/cmd, A0, register select) pin: Output level in arg_int
            break;
        case U8X8_MSG_GPIO_RESET:			// Reset pin: Output level in arg_int
            break;
        case U8X8_MSG_GPIO_CS1:				// CS1 (chip select) pin: Output level in arg_int
            break;
        case U8X8_MSG_GPIO_CS2:				// CS2 (chip select) pin: Output level in arg_int
            break;
        case U8X8_MSG_GPIO_I2C_CLOCK:		// arg_int=0: Output low at I2C clock pin
            break;							// arg_int=1: Input dir with pullup high for I2C clock pin
        case U8X8_MSG_GPIO_I2C_DATA:		// arg_int=0: Output low at I2C data pin
            break;							// arg_int=1: Input dir with pullup high for I2C data pin
        case U8X8_MSG_GPIO_MENU_SELECT:
            u8x8_SetGPIOResult(u8x8, /* get menu select pin state */ 0);
            break;
        case U8X8_MSG_GPIO_MENU_NEXT:
            u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
            break;
        case U8X8_MSG_GPIO_MENU_PREV:
            u8x8_SetGPIOResult(u8x8, /* get menu prev pin state */ 0);
            break;
        case U8X8_MSG_GPIO_MENU_HOME:
            u8x8_SetGPIOResult(u8x8, /* get menu home pin state */ 0);
            break;
        default:
            u8x8_SetGPIOResult(u8x8, 1);			// default return value
            break;
    }
    return 1;
};

uint8_t U8G2::u8x8_byte_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    U8G2 * instance = (U8G2 *)u8x8_GetUserPtr(u8x8);
    switch(msg)
    {
        case U8X8_MSG_BYTE_SEND:
        {   
            uint8_t *data = (uint8_t *)arg_ptr;   
            while( arg_int > 0 )
            {
                instance->buffer[instance->buffer_length++] = *data;
                data++;
                arg_int--;
            }  
            break;
        }
        case U8X8_MSG_BYTE_INIT:
            break;
        case U8X8_MSG_BYTE_SET_DC:
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:
        {
            instance->i2c_master->Write(instance->i2c_device_address, 
                                        instance->buffer, 
                                        instance->buffer_length);
            instance->buffer_length = 0;
            break;
        }
        default:
            return 0;
    }
    return 1;
}

U8G2::U8G2(I2cMaster* const i2c_master, 
           const uint8_t i2c_device_address, 
           const DeviceType device_type, 
           const u8g2_cb_t* rotation)
{
    this->mutex = xSemaphoreCreateMutex();
    this->i2c_master = i2c_master;
    this->i2c_device_address = i2c_device_address;
    switch (device_type)
    {
        case DeviceType::SSD1306_I2C_128x64:
            u8g2_Setup_ssd1306_i2c_128x64_noname_f(&instance, 
                                                   rotation, 
                                                   U8G2::u8x8_byte_i2c, 
                                                   U8G2::u8x8_gpio_and_delay);
            break;
        case DeviceType::SH1106_I2C_128x64:
            u8g2_Setup_sh1106_i2c_128x64_noname_f(&instance, 
                                                  rotation, 
                                                  U8G2::u8x8_byte_i2c, 
                                                  U8G2::u8x8_gpio_and_delay);
            break;
        default:
            ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
            break;
    }
    auto buf = (uint8_t*)malloc(u8g2_GetBufferSize(&instance));
	u8g2_SetBufferPtr(&instance, buf);
    u8g2_SetUserPtr(&instance, this);
    u8g2_InitDisplay(&instance);
    u8g2_SetPowerSave(&instance, 0);
    u8g2_ClearBuffer(&instance);
    u8g2_ClearDisplay(&instance);
}

U8G2::~U8G2()
{   
    free(u8g2_GetBufferPtr(&instance));
}

u8g2_t * U8G2::GetInstance()
{
    return &instance;
}

void U8G2::Lock()
{
    // 设置临界区
    xSemaphoreTake(this->mutex, portMAX_DELAY);
    return;
}
        
void U8G2::Unlock()
{
    // 退出临界区
    xSemaphoreGive(this->mutex);
    return;
}

}

}
