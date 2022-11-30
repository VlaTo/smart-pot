#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/gpio.h>

#ifdef CONFIG_SMARTPOT_USING_I2C

#include "i2c_bus.h"
#include "ssd1306_device.h"
#include "ds3231_device.h"

#endif

static const char* TAG = "smart-pot";

#ifdef CONFIG_SMARTPOT_USING_I2C
#else

#endif

#ifdef CONFIG_SMARTPOT_USING_I2C

esp_err_t i2c_master_send(uint8_t message[], int len);

#endif

void app_main(void)
{
    ESP_LOGI(TAG, "Starting app_main");
#ifdef CONFIG_SMARTPOT_USING_I2C
    //ssd1306_device_t ssd1306;
    ESP_ERROR_CHECK(i2c_master_init(I2C_NUM_1));

    ds3231_device_t ds3231;
    //ESP_ERROR_CHECK(ssd1306_device_init(&ssd1306, DEFAULT_SSD1306_ADDR));
    ESP_ERROR_CHECK(ds3231_device_init(&ds3231, DEFAULT_DS3231_ADDR));

    struct tm ti /*= {
        .tm_sec = 23,
        .tm_min = 32,
        .tm_hour = 12,
        .tm_wday = 3,
        .tm_mday = 30,
        .tm_mon = 11,
        .tm_year = 2022
    }*/;
    float temp;
    char str[64];

    memset(str, 0, sizeof(str));
    //ESP_ERROR_CHECK(ds3231_set_time(&ds3231, &ti));
    ESP_ERROR_CHECK(ds3231_get_temp_float(&ds3231, &temp));
    ESP_LOGI(TAG, "Temp: %f", temp);

    while(true) {
        ESP_ERROR_CHECK(ds3231_get_time(&ds3231, &ti));
        strftime(str, sizeof(str), "%c", &ti);
        ESP_LOGI(TAG, "DS3231 date/time: %s", str);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
#else
    ;
#endif
}

#ifdef CONFIG_SMARTPOT_USING_I2C


#else

#endif