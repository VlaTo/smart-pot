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
#undef U8X8_USE_PINS
#include "u8g2.h"
#include "u8x8.h"
#include "u8g2_esp32_hal.h"
#include "ds3231.hpp"
#include "at24cxx.hpp"
#include "main.hpp"

#define I2C_PORT1_SDA_PIN                   (GPIO_NUM_21)
#define I2C_PORT1_SCL_PIN                   (GPIO_NUM_22)

static const char* TAG = "smart-pot";

// forward functions declaration
/*static void smartpot_i2c_init();
static void smartpot_display_init();
static void smartpot_rtc_init();
static void smartpot_nop();
static void smartpot_try_to_water();
static void smartpot_enter_menu();
*/
u8g2_t u8g2 = {};
Ds3231 rtc(I2C_MASTER_NUM, DS3231_ADDR);
At24cxx eeprom(I2C_MASTER_NUM, AT24CXX_ADDR, CHIP_8x4096);

smartpot_action_cb main_action = NULL;

/*
#define CONFIG_EXAMPLE_I2C_MASTER_SDA       (GPIO_NUM_21)
#define CONFIG_EXAMPLE_I2C_MASTER_SCL       (GPIO_NUM_22)
#define CONFIG_EXAMPLE_I2C_CLOCK_HZ         400000

void task(void *ignore)
{
    i2c_dev_t dev = { 0 };
    dev.port = I2C_NUM_1;
    dev.cfg.mode = I2C_MODE_MASTER;
    dev.cfg.sda_io_num = CONFIG_EXAMPLE_I2C_MASTER_SDA;
    dev.cfg.scl_io_num = CONFIG_EXAMPLE_I2C_MASTER_SCL;
#if HELPER_TARGET_IS_ESP32
    dev.cfg.master.clk_speed = CONFIG_EXAMPLE_I2C_CLOCK_HZ; // 400kHz
#endif
    while (1)
    {
        esp_err_t res;
        printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
        printf("00:         ");
        for (uint8_t addr = 3; addr < 0x78; addr++)
        {
            if (addr % 16 == 0)
                printf("\n%.2x:", addr);

            dev.addr = addr;
            res = i2c_dev_probe(&dev, I2C_DEV_WRITE);

            if (res == 0)
                printf(" %.2x", addr);
            else
                printf(" --");
        }
        printf("\n\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

extern "C" void app_main(void)
{
    // Init i2cdev library
    ESP_ERROR_CHECK(i2c_dev_init());
    // Start task
    xTaskCreate(task, "i2c_scanner", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
}
*/
extern "C" void app_main(void)
{
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;

    u8g2_esp32_hal.sda = I2C_PORT1_SDA_PIN;
    u8g2_esp32_hal.scl = I2C_PORT1_SCL_PIN;

    u8g2_esp32_hal_init(u8g2_esp32_hal);    
    
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8g2_esp32_i2c_byte_cb, u8g2_esp32_gpio_and_delay_cb);
    u8x8_SetI2CAddress(&u8g2.u8x8, 0x3C);
    
    ESP_LOGI(TAG, "u8g2_InitDisplay");
	u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,

    uint8_t mem[16];
    eeprom.read(0, mem, sizeof(mem));

    ESP_LOGI(TAG, "u8g2_SetPowerSave");
	u8g2_SetPowerSave(&u8g2, 0); // wake up display
	ESP_LOGI(TAG, "u8g2_ClearBuffer");
	u8g2_ClearBuffer(&u8g2);

	ESP_LOGI(TAG, "u8g2_SetFont");
    u8g2_SetFont(&u8g2, u8g2_font_7x13_t_cyrillic);
	ESP_LOGI(TAG, "u8g2_DrawStr");
    u8g2_DrawStr(&u8g2, 2, 17, "\u0411...");

    struct tm time;
    uint16_t _value = 0;
    char str[64];
    while(1)
    {
        rtc.get_time(&time);
        strftime(str, sizeof(str), "%c", &time);
        ESP_LOGI(TAG, "DS3231 date/time %s", str);

        u8g2.draw_color = 1;
        ESP_LOGI(TAG, "u8g2_DrawBox");
        u8g2_DrawFrame(&u8g2, 0, 26, 102, 10);
        u8g2.draw_color = 0;
        u8g2_DrawBox(&u8g2, 1, 27, 100, 8);
        uint16_t temp = (_value * 100) / 100;
        u8g2.draw_color = 1;
        u8g2_DrawBox(&u8g2, 1, 27, temp, 8);

        ESP_LOGI(TAG, "u8g2_SendBuffer");
        u8g2_SendBuffer(&u8g2);

        _value = (_value + 5) % 105;

        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    ESP_LOGI(TAG, "All done!");
}

/*
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
    // u8x8_d_ssd1306_128x64_noname()
    //u8g2_Setup_ssd1306_128x64_noname_f
	//u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8g2_esp32_i2c_byte_cb, u8g2_esp32_gpio_and_delay_cb);
    //u8g2_Setup_ssd1306_i2c_128x64_noname_2(&u8g2, U8G2_R0, u8g2_esp32_i2c_byte_cb, u8g2_esp32_gpio_and_delay_cb);
    u8g2_Setup_ssd1306_i2c_128x64_noname_1(&u8g2, U8G2_R0, u8g2_esp32_i2c_byte_cb, u8g2_esp32_gpio_and_delay_cb);
    u8x8_SetI2CAddress(&u8g2.u8x8, SSD1306_ADDR);
	u8g2_InitDisplay(&u8g2);
	u8g2_SetPowerSave(&u8g2, 0);
}

static void smartpot_rtc_init()
{
    rtc.timeout_ticks = pdMS_TO_TICKS(CONFIG_I2CDEV_TIMEOUT);
    rtc.cfg.mode = I2C_MODE_MASTER;
    rtc.cfg.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;
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
*/