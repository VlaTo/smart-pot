#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_event.h"
#include "ds3231.hpp"
#include "at24cxx.hpp"
#include "smartpot.hpp"
#include "encoder.h"

static const char* TAG = "smart-pot";

/**
 * 
 * 
 */
MenuItem::MenuItem(const char* title, int tag, menu_item_cb cb)
    : has_sub_menu(0), title(title), tag(tag), cb(cb)
{
}

MenuItem::MenuItem(const char* title, MenuItem* sub_menu)
    : has_sub_menu(1), title(title), tag(-1), sub_menu(sub_menu)
{
}

MenuItem::~MenuItem()
{
}

/**
 * 
 */
Menu::Menu(MenuItem* menu_item)
    : menu_item(menu_item), top_item(menu_item)
{    
}

Menu::~Menu()
{    
}

/**
 * 
 * 
 */
SmartPot::SmartPot(u8g2_t* u8g2, Ds3231* rtc, At24cxx* eeprom, Dht* dht)
    : display(u8g2), rtc(rtc), eeprom(eeprom), dht(dht)
{
    _init();
}

SmartPot::~SmartPot()
{

}

void SmartPot::begin(esp_sleep_wakeup_cause_t wakeup_cause)
{
    switch (wakeup_cause)
    {
        case ESP_SLEEP_WAKEUP_EXT0: {
            //main_action = smartpot_try_to_water;
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
            //main_action = smartpot_enter_menu;
            break;
        }        

        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default: {
            ESP_LOGI(TAG, "Wake up by undefined cause");
            //main_action = smartpot_nop;
            break;
        }
    }
}

esp_err_t SmartPot::run(QueueHandle_t* queue)
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    uint16_t line_height = u8g2_GetAscent(display) - u8g2_GetDescent(display) + 2;
    uint16_t top = line_height;

    display->draw_color = 1;
    u8g2_DrawUTF8(display, 2, top, "Тек. состояние");
    top += line_height;
    u8g2_DrawUTF8(display, 2, top, "Подкл. к WIFI");
    top += line_height;
    u8g2_DrawUTF8(display, 2, top, "Настройки");
    u8g2_SendBuffer(display);

    struct tm time;
    uint16_t _value = 0;
    char str[64];
    float humidity, temperature;
    rotary_encoder_event_t e;
    int32_t val = 0;
    while(1)
    {
        //xQueueReceive(*queue, &e, pdMS_TO_TICKS(1000));
        xQueueReceive(*queue, &e, portMAX_DELAY);

        switch(e.type)
        {
            case RE_ET_BTN_PRESSED:
            {
                ESP_LOGI(TAG, "Button pressed");
                break;
            }
            case RE_ET_BTN_RELEASED:
            {
                ESP_LOGI(TAG, "Button released");
                break;
            }
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
                val += e.diff;
                ESP_LOGI(TAG, "Value = %" PRIi32, val);
                break;
            }
            default:
            {
                break;
            }
        }

        ESP_ERROR_CHECK(rtc->get_time(&time));
        strftime(str, sizeof(str), "%c", &time);
        ESP_LOGI(TAG, "DS3231 date/time %s", str);

        //ESP_ERROR_CHECK(dht->read_float_data(&humidity, &temperature));
        //ESP_LOGI(TAG, "DHT hum %.1f%% temp %.1fC", humidity, temperature);

        /*display->draw_color = 1;
        u8g2_DrawFrame(display, 0, 26, 102, 10);
        display->draw_color = 0;
        u8g2_DrawBox(display, 1, 27, 100, 8);
        uint16_t temp = (_value * 100) / 100;
        display->draw_color = 1;
        u8g2_DrawBox(display, 1, 27, temp, 8);
        u8g2_SendBuffer(display);

        _value = (_value + 5) % 105;*/

        //vTaskDelay(pdMS_TO_TICKS(1000));
    }

    return ESP_OK;
}

void SmartPot::handle(int val)
{

}

void SmartPot::_init()
{
    ;
}