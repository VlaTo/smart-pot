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
#include "esp_timer.h"
#include "esp_event.h"
#undef U8X8_USE_PINS
#define U8G2_REF_MAN_PIC
#include "u8g2.h"
#include "u8x8.h"
#include "u8g2_esp32_hal.h"
#include "encoder.h"
#include "ds3231.hpp"
#include "at24cxx.hpp"
#include "main.hpp"
#include "smartpot.hpp"
#include "dht.hpp"
#include "menu_item.hpp"
#include "menu.hpp"
#include "moisture_sensor.hpp"

#define I2C_PORT1_SDA_PIN                   (GPIO_NUM_21)
#define I2C_PORT1_SCL_PIN                   (GPIO_NUM_22)
#define DHT_ONE_WIRE_PIN                    (GPIO_NUM_19)
#define RE_CLK_PIN                          (GPIO_NUM_17)
#define RE_DT_PIN                           (GPIO_NUM_16)
#define RE_SW_PIN                           (GPIO_NUM_4)
#define MS_DATA_PIN                         (GPIO_NUM_15)
#define MS_POWER_PIN                        (GPIO_NUM_2)
#define EV_QUEUE_LEN                        (5)

static const char* TAG = "main";

static const char* mons[] = {
    "января",
    "февраля",
    "марта",
    "апреля",
    "мая",
    "июня",
    "июля",
    "августа",
    "сентября",
    "октября",
    "ноября",
    "декабря"
};

typedef enum
{
    CONTRAST_NOP = 0,
    CONTRAST_HIGH,
    CONTRAST_LOW
} contrast_action_t;

typedef struct
{
    float aerial_humidity;
    float aerial_temperature;
    int soil_moisture; 
    TickType_t ticks;
} smartpot_sensors_event_t;

/*typedef struct
{
    u8g2_t* display;
    QueueHandle_t* queue;    
} smartpot_output_task_args_t;*/

u8g2_t u8g2 = {};
u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
Ds3231 rtc(I2C_MASTER_NUM, DS3231_ADDR);
At24cxx eeprom(I2C_MASTER_NUM, AT24CXX_ADDR, CHIP_8x4096);
Dht aerial_sensor(DHT_ONE_WIRE_PIN, DHT_TYPE_DHT11);
MoistureSensor moisture_sensor(MS_DATA_PIN, MS_POWER_PIN);
rotary_encoder_t rotary_encoder;
QueueHandle_t sensor_queue;

void smartpot_read_sensors(void* extra_data);
void smartpot_write_sensors(void* extra_data);
void smartpot_main_screen(void* args);

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
    //int64_t microseconds = esp_timer_get_time();
    //TickType_t startTicks = xTaskGetTickCount();
    
    // get MCU's wakeup cause
    esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();

    // Create event queue for rotary encoders
    QueueHandle_t event_queue = xQueueCreate(EV_QUEUE_LEN, sizeof(rotary_encoder_event_t));
    // Setup rotary encoder library
    ESP_ERROR_CHECK(rotary_encoder_init(event_queue));
    memset(&rotary_encoder, 0, sizeof(rotary_encoder_t));
    rotary_encoder.pin_a = RE_CLK_PIN;
    rotary_encoder.pin_b = RE_DT_PIN;
    rotary_encoder.pin_btn = RE_SW_PIN;
    ESP_ERROR_CHECK(rotary_encoder_add(&rotary_encoder));

    // Setup U8G2 library
    u8g2_esp32_hal.sda = I2C_PORT1_SDA_PIN;
    u8g2_esp32_hal.scl = I2C_PORT1_SCL_PIN;
    u8g2_esp32_hal_init(u8g2_esp32_hal);    
    //u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8g2_esp32_i2c_byte_cb, u8g2_esp32_gpio_and_delay_cb);
    u8g2_Setup_sh1106_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8g2_esp32_i2c_byte_cb, u8g2_esp32_gpio_and_delay_cb);
    u8x8_SetI2CAddress(&u8g2.u8x8, 0x3C);
	u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,
	u8g2_SetPowerSave(&u8g2, 0); // wake up display
    //u8g2_SetFont(&u8g2, u8g2_font_9x15_t_cyrillic); // u8g2_font_8x13_t_cyrillic u8g2_font_9x15_t_cyrillic
    //u8g2_SetFont(&u8g2, u8g2_font_8x13_t_cyrillic); // u8g2_font_8x13_t_cyrillic u8g2_font_9x15_t_cyrillic
    
    // Check EEPROM memory
    uint8_t mem[16];
    eeprom.read(0, mem, sizeof(mem));
    ESP_LOG_BUFFER_HEXDUMP(TAG, mem, sizeof(mem), ESP_LOG_INFO);
    
    // Setup analog moisture sensor
    moisture_sensor.setup();
    
    struct tm time_from_rtc;
    esp_err_t result = rtc.get_time(&time_from_rtc);
    if (ESP_OK == result)
    {
        time_t converted_time = mktime(&time_from_rtc);
        struct timeval time_of_day = {
            .tv_sec = converted_time,
            .tv_usec = 0llu
        };
        settimeofday(&time_of_day, NULL);
        setenv("TZ", "MSK-3", 1);
        tzset();
    }

    /*sensor_queue = xQueueCreate(EV_QUEUE_LEN, sizeof(smartpot_sensors_event_t));

    TaskHandle_t sensor_reading_task;
    xTaskCreate(
        smartpot_read_sensors,
        "smartpot_read_sensors",
        4 * 1024,
        &sensor_queue,
        0,
        &sensor_reading_task
    );

    TaskHandle_t sensor_writing_task;
    xTaskCreate(
        smartpot_write_sensors,
        "smartpot_write_sensors",
        8 * 1024,
        &sensor_queue,
        0,
        &sensor_writing_task
    );*/

    TaskHandle_t main_screen_task;
    xTaskCreate(
        smartpot_main_screen,
        "smartpot_main_screen",
        8 * 1024,
        &event_queue,
        0,
        &main_screen_task
    );
    
    //smartpot->begin(wakeup_cause);

	//u8g2_ClearBuffer(&u8g2);
    //u8g2_SetFont(&u8g2, u8g2_font_9x15_t_cyrillic); // u8g2_font_8x13_t_cyrillic u8g2_font_9x15_t_cyrillic

    //uint16_t line_height = u8g2_GetAscent(&u8g2) - u8g2_GetDescent(&u8g2);
    //u8g2_DrawUTF8(&u8g2, 0, line_height, "Поливалка");
    //u8g2_SendBuffer(&u8g2);

    //u8g2_UserInterfaceSelectionList(&u8g2, "Title", 0, "abc\ndef\nghi\njkl\n12345\n67890");

    MenuItem curr_menu_item("Тек. остояние", (menu_item_cb)NULL);
    MenuItem wifi_menu_item("Подкл. к WIFI", (menu_item_cb)NULL);
    MenuItem settings_menu_item("Настройки", (menu_item_cb)NULL);
    MenuItem* top_menu[] = { &curr_menu_item, &wifi_menu_item, &settings_menu_item };
    
    Menu menu(top_menu, 3);

    //MenuItem* temp = menu[0];

    //menu[0] = menu_item;

    //smartpot->run(&event_queue);

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }

    ESP_LOGI(TAG, "All done!");
}

esp_err_t read_aerial_humidity_temperature(float* aerial_humidity, float* aerial_temperature)
{
    const TickType_t delay_ticks = pdMS_TO_TICKS(2500);
    int count = 0;
    float humidity_acc = 0, temperature_acc = 0;
    for (int pass = 10; 0 < pass && 3 > count; pass--)
    {
        float humidity, temperature;
        esp_err_t result = aerial_sensor.read_float_data(&humidity, &temperature);
        if (ESP_OK == result)
        {
            humidity_acc += humidity;
            temperature_acc += temperature;
            count++;
        }

        vTaskDelay(delay_ticks);
    }

    if (0 < count)
    {
        *aerial_humidity = humidity_acc / count;
        *aerial_temperature = temperature_acc / count;

        return ESP_OK;
    }

    return ESP_FAIL;
}

esp_err_t read_soil_moisture(int* soil_moisture)
{
    const TickType_t delay_ticks = pdMS_TO_TICKS(500);
    int soil_moisture_acc = 0, count = 0;
    for (int pass = 3; 0 < pass; pass--)
    {
        int voltage;
        esp_err_t result = moisture_sensor.read_voltage(&voltage);
        if (ESP_OK == result)
        {
            soil_moisture_acc += voltage;
            count++;
        }

        vTaskDelay(delay_ticks);
    }

    if (0 < count)
    {
        *soil_moisture = (int)(soil_moisture_acc / count);
        return ESP_OK;
    }

    return ESP_FAIL;
}

void smartpot_write_sensors(void* extra_data)
{
    QueueHandle_t* queue = (QueueHandle_t*)extra_data;
    smartpot_sensors_event_t ev;
    char line[64];
    uint16_t pass = 0;
    
    u8g2_SetFont(&u8g2, u8g2_font_8x13_t_cyrillic); // u8g2_font_8x13_t_cyrillic u8g2_font_9x15_t_cyrillic	
    uint16_t line_height = u8g2_GetAscent(&u8g2) - u8g2_GetDescent(&u8g2);

    u8g2_ClearBuffer(&u8g2);
    u8g2_DrawUTF8(&u8g2, 0, line_height, "Ожидание...");
    u8g2_SendBuffer(&u8g2);

    // void u8g2_SetFontRefHeightText(u8g2_t *u8g2);
    // u8g2_uint_t u8g2_GetUTF8Width(u8g2_t *u8g2, const char *str);
    // #define u8g2_GetMaxCharHeight(u8g2) ((u8g2)->font_info.max_char_height)
    // #define u8g2_GetMaxCharWidth(u8g2) ((u8g2)->font_info.max_char_width)
    // #define u8g2_GetAscent(u8g2) ((u8g2)->font_ref_ascent)
    // #define u8g2_GetDescent(u8g2) ((u8g2)->font_ref_descent)
    // #define u8g2_GetFontAscent(u8g2) ((u8g2)->font_ref_ascent)
    // #define u8g2_GetFontDescent(u8g2) ((u8g2)->font_ref_descent)

    while(1)
    {
        xQueueReceive(*queue, &ev, portMAX_DELAY);

        u8g2_ClearBuffer(&u8g2);
        sprintf(line, "Влажн: %.1f%%", ev.aerial_humidity);
        ESP_LOGI(TAG, "%s", line);
        u8g2_DrawUTF8(&u8g2, 0, line_height, line);
        sprintf(line, "Темпр: %.1fC", ev.aerial_temperature);
        ESP_LOGI(TAG, "%s", line);
        u8g2_DrawUTF8(&u8g2, 0, line_height * 2, line);
        sprintf(line, "Почва: %dmV", ev.soil_moisture);
        ESP_LOGI(TAG, "%s", line);
        u8g2_DrawUTF8(&u8g2, 0, line_height * 3, line);
        sprintf(line, "Отсчт: %lu", ev.ticks);
        ESP_LOGI(TAG, "%s", line);
        u8g2_DrawUTF8(&u8g2, 0, line_height * 4, line);
        sprintf(line, "Кадр : %d", ++pass);
        ESP_LOGI(TAG, "%s", line);
        u8g2_DrawUTF8(&u8g2, 0, line_height * 5, line);
        u8g2_SendBuffer(&u8g2);
    }
}

/*

*/
void smartpot_read_sensors(void* extra_data)
{
    //MoistureSensor* moisture_sensor = new MoistureSensor(MS_DATA_PIN, MS_POWER_PIN);
    //Dht* aerial_sensor = new Dht(DHT_ONE_WIRE_PIN, DHT_TYPE_DHT11);
    QueueHandle_t* queue = (QueueHandle_t*)extra_data;
    smartpot_sensors_event_t ev = { 0 };   

    while (1)
    {
        float aerial_humidity, aerial_temperature;
        esp_err_t result = read_aerial_humidity_temperature(&aerial_humidity, &aerial_temperature);
        if (ESP_OK != result)
        {
            ;
        }
        
        int soil_moisture;
        result = read_soil_moisture(&soil_moisture);
        if (ESP_OK != result)
        {
            ;
        }

        ev.aerial_humidity = aerial_humidity;
        ev.aerial_temperature = aerial_temperature;
        ev.soil_moisture = soil_moisture;
        ev.ticks = xTaskGetTickCount();
        xQueueSendToBack(*queue, &ev, 0);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

static void smartpot_main_screen()
{

}

void smartpot_main_screen(void* args)
{
    const TickType_t delay_ticks = pdMS_TO_TICKS(1000);
    TickType_t last_activation_ticks = xTaskGetTickCount();
    QueueHandle_t* queue = (QueueHandle_t*)args;
    int8_t screen_index = 0, screen_count = 2;
    rotary_encoder_event_t e;
    char short_str[24];
	uint8_t contrast_action = CONTRAST_HIGH;
    
    u8g2_SetContrast(&u8g2, 255);
    u8g2_ClearBuffer(&u8g2);

    while (1)
    {
        BaseType_t result = xQueueReceive(*queue, &e, delay_ticks);
        TickType_t current_ticks = xTaskGetTickCount();

        if (pdTRUE == result)
        {
            last_activation_ticks = current_ticks;
            if (CONTRAST_LOW == contrast_action)
            {
                u8g2_SetContrast(&u8g2, 255);
                contrast_action = CONTRAST_HIGH;
            }

            switch (e.type)
            {
                /*case RE_ET_BTN_PRESSED:
                {
                    ESP_LOGI(TAG, "Button pressed");
                    break;
                }
                case RE_ET_BTN_RELEASED:
                {
                    ESP_LOGI(TAG, "Button released");
                    break;
                }*/
                case RE_ET_BTN_CLICKED:
                {
                    ESP_LOGI(TAG, "Button clicked");
                    break;
                }
                case RE_ET_BTN_LONG_PRESSED:
                {
                    ESP_LOGI(TAG, "Looooong pressed button");
                    break;
                }
                case RE_ET_CHANGED:
                {
                    int8_t new_screen_index = screen_index + e.diff;
                    if (0 > new_screen_index)
                    {
                        screen_index = screen_count - 1;
                    }
                    else if ((screen_count - 1) < new_screen_index)
                    {
                        screen_index = 0;
                    }
                    else
                    {
                        screen_index = new_screen_index;
                    }

                    //ESP_LOGI(TAG, "Value = %" PRIi32, position);
                    break;
                }
                default:
                {
                    ESP_LOGI(TAG, "Unknown event type");
                    break;
                }
            }
        }
        else
        {
            TickType_t idle_ms = pdTICKS_TO_MS(current_ticks - last_activation_ticks);
            if (10000 < idle_ms && CONTRAST_HIGH == contrast_action)
            {
                contrast_action = CONTRAST_LOW;
                u8g2_SetContrast(&u8g2, 20);
            }
        }

        u8g2_uint_t screen_height = u8g2_GetDisplayHeight(&u8g2);
        u8g2_uint_t screen_width = u8g2_GetDisplayWidth(&u8g2);
        
        u8g2_ClearBuffer(&u8g2);

        switch (screen_index)
        {
            case 0:
            {
                time_t system_time;
                struct tm local_time;
                time(&system_time);
                localtime_r(&system_time, &local_time);
                sprintf(short_str, "%02d:%02d", local_time.tm_hour, local_time.tm_min);
                u8g2_SetFont(&u8g2, u8g2_font_timR24_tn);
                u8g2_uint_t str_height = u8g2_GetMaxCharHeight(&u8g2);
                u8g2_uint_t x = (screen_width - u8g2_GetUTF8Width(&u8g2, short_str)) / 2;
                u8g2_uint_t y = (screen_height - str_height) / 2 + str_height - 4;
                u8g2_DrawUTF8(&u8g2, x, y, short_str);
                u8g2_SetFont(&u8g2, u8g2_font_7x13_t_cyrillic);
                sprintf(short_str, "%d %s %04d", local_time.tm_mday, mons[local_time.tm_mon], local_time.tm_year);
                str_height = u8g2_GetMaxCharHeight(&u8g2);
                x = (screen_width - u8g2_GetUTF8Width(&u8g2, short_str)) / 2;
                y += (str_height + 4);
                u8g2_DrawUTF8(&u8g2, x, y, short_str);

                break;
            }

            case 1:
            {
                u8g2_SetFont(&u8g2, u8g2_font_7x13_t_cyrillic);
                u8g2_uint_t y = u8g2_GetMaxCharHeight(&u8g2);
                u8g2_DrawUTF8(&u8g2, 4, y, "Тек. состояние:");

                break;
            }
        }

        // write bottom page index
        u8g2_SetFont(&u8g2, u8g2_font_5x8_mn);
        sprintf(short_str, "%d/%d", screen_index + 1, screen_count);        
        u8g2_uint_t str_width = u8g2_GetUTF8Width(&u8g2, short_str);
        u8g2_uint_t str_height = u8g2_GetMaxCharHeight(&u8g2);
        u8g2_DrawUTF8(&u8g2, screen_width - str_width, str_height, short_str);

        u8g2_SendBuffer(&u8g2);
    }
}