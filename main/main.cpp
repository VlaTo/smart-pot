#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_event.h"
#include "i2cdev.h"
#include "u8g2.h"
#include "u8x8.h"
#include "u8g2_esp32_hal.h"
#include "ds3231.h"
#include "main.hpp"

#define I2C_PORT1_SDA_PIN                   (GPIO_NUM_21)
#define I2C_PORT1_SCL_PIN                   (GPIO_NUM_22)

typedef void (*smartpot_main_action)();

static const char* TAG = "smart-pot";

// forward functions declaration
static void smartpot_i2c_init();
static void smartpot_display_init();
static void smartpot_rtc_init();
static void smartpot_nop();
static void smartpot_try_to_water();
static void smartpot_enter_menu();

u8g2_t u8g2 = {};
i2c_dev_t rtc = {};
smartpot_main_action main_action = NULL;

extern "C" void app_main(void)
{
    esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
    switch (wakeup_cause)
    {
        case ESP_SLEEP_WAKEUP_EXT0: {
            main_action = smartpot_try_to_water;
            break;
        }

        case ESP_SLEEP_WAKEUP_EXT1: {
            uint64_t wakeup_pin_mask = esp_sleep_get_ext1_wakeup_status();
            if (0 != wakeup_pin_mask) {
                int pin = __builtin_ffsll(wakeup_pin_mask) - 1;
                ESP_LOGI(TAG, "Wake up from GPIO num %d", pin);
            } else {
                ESP_LOGI(TAG, "Wake up from GPIO");
            }
            main_action = smartpot_enter_menu;
            break;
        }        

        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default: {
            ESP_LOGI(TAG, "Wake up by undefined cause");
            main_action = smartpot_nop;
            break;
        }
    }

    // initialization
    smartpot_i2c_init();
    smartpot_display_init();
    smartpot_rtc_init();

    // run
    main_action();
}

static void smartpot_i2c_init()
{
    i2c_dev_init();
}

static void smartpot_display_init()
{
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;

    u8g2_esp32_hal.sda = I2C_PORT1_SDA_PIN;
    u8g2_esp32_hal.scl = I2C_PORT1_SCL_PIN;

    u8g2_esp32_hal_init(u8g2_esp32_hal);    
    /*u8x8_d_ssd1306_128x64_noname()*/
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8g2_esp32_i2c_byte_cb, u8g2_esp32_gpio_and_delay_cb);    
    u8x8_SetI2CAddress(&u8g2.u8x8, SSD1306_ADDR);
	u8g2_InitDisplay(&u8g2);
	u8g2_SetPowerSave(&u8g2, 0);
}

static void smartpot_rtc_init()
{
    ESP_ERROR_CHECK(ds3231_init_desc(&rtc, I2C_NUM_1, I2C_PORT1_SDA_PIN, I2C_PORT1_SCL_PIN));
}

static void smartpot_nop()
{
    ESP_LOGI(TAG, "Smartpot nop");

    struct tm time;
    char str[64];

    while(1)
    {
        ESP_ERROR_CHECK(ds3231_get_time(&rtc, &time));
        strftime(str, sizeof(str), "%c", &time);
        ESP_LOGI(TAG, "DS3231 date/time %s", str);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void smartpot_try_to_water()
{
    ESP_LOGI(TAG, "Smartpot try to water");
    ESP_ERROR_CHECK(esp_event_loop_create_default());
}

static void smartpot_enter_menu()
{
    ESP_LOGI(TAG, "Wake up by undefined cause");
}