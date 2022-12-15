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
#include "encoder.h"
#include "ds3231.hpp"
#include "at24cxx.hpp"
#include "main.hpp"
#include "smartpot.hpp"
#include "dht.hpp"

#define I2C_PORT1_SDA_PIN                   (GPIO_NUM_21)
#define I2C_PORT1_SCL_PIN                   (GPIO_NUM_22)
#define DHT_ONE_WIRE_PIN                    (GPIO_NUM_19)
#define RE_CLK_PIN                          (GPIO_NUM_17)
#define RE_DT_PIN                           (GPIO_NUM_16)
#define RE_SW_PIN                           (GPIO_NUM_4)
#define EV_QUEUE_LEN                        (5)

static const char* TAG = "main";

u8g2_t u8g2 = {};
//Ds3231 rtc(I2C_MASTER_NUM, DS3231_ADDR);
//At24cxx eeprom(I2C_MASTER_NUM, AT24CXX_ADDR, CHIP_8x4096);

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
    Ds3231* rtc = new Ds3231(I2C_MASTER_NUM, DS3231_ADDR);
    At24cxx* eeprom = new At24cxx(I2C_MASTER_NUM, AT24CXX_ADDR, CHIP_8x4096);
    Dht* dht = new Dht(DHT_ONE_WIRE_PIN, DHT_TYPE_DHT11);
    rotary_encoder_t re;
    SmartPot* smartpot = new SmartPot(&u8g2, rtc, eeprom, dht);
    esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();

    // Create event queue for rotary encoders
    QueueHandle_t event_queue = xQueueCreate(EV_QUEUE_LEN, sizeof(rotary_encoder_event_t));
    // Setup rotary encoder library
    ESP_ERROR_CHECK(rotary_encoder_init(event_queue));
    // 
    memset(&re, 0, sizeof(rotary_encoder_t));
    re.pin_a = RE_CLK_PIN;
    re.pin_b = RE_DT_PIN;
    re.pin_btn = RE_SW_PIN;
    ESP_ERROR_CHECK(rotary_encoder_add(&re));

    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;

    u8g2_esp32_hal.sda = I2C_PORT1_SDA_PIN;
    u8g2_esp32_hal.scl = I2C_PORT1_SCL_PIN;

    u8g2_esp32_hal_init(u8g2_esp32_hal);    
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8g2_esp32_i2c_byte_cb, u8g2_esp32_gpio_and_delay_cb);
    u8x8_SetI2CAddress(&u8g2.u8x8, 0x3C);
	u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,
	u8g2_SetPowerSave(&u8g2, 0); // wake up display

    uint8_t mem[16];
    eeprom->read(0, mem, sizeof(mem));
    ESP_LOG_BUFFER_HEXDUMP(TAG, mem, sizeof(mem), ESP_LOG_INFO);
    
    smartpot->begin(wakeup_cause);

	u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_9x15_t_cyrillic); // u8g2_font_8x13_t_cyrillic u8g2_font_9x15_t_cyrillic
    
    const MenuItem* temp[] = {
        new MenuItem("Тек. остояние", 1, &SmartPot::handle),
        new MenuItem("Подкл. к WIFI", 2, &SmartPot::handle),
        new MenuItem("Настройки", 3, &SmartPot::handle),
    };

    smartpot->run(&event_queue);

    ESP_LOGI(TAG, "All done!");
}