/**
 * 
 * 
 * 
 * 
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "esp_sleep.h"
#include "u8g2.h"
#include "ds3231.hpp"
#include "dht.hpp"

class SmartPot;

typedef void (SmartPot::* menu_item_cb)(int);

class MenuItem final
{
public:
    MenuItem(const char* title, int tag, menu_item_cb cb);
    MenuItem(const char* title, MenuItem* sub_menu);
    ~MenuItem();

private:

    bool has_sub_menu;
    const char* title;
    int tag;
    union
    {
        menu_item_cb cb;
        MenuItem* sub_menu;
    };
};

/**
 * 
 */
class Menu final
{
public:
    Menu(MenuItem* menu_item);
    ~Menu();

private:
    MenuItem* menu_item;
    MenuItem* top_item;
};

class SmartPot final
{
public:
    SmartPot(u8g2_t* u8g2, Ds3231* rtc, At24cxx* eeprom, Dht* dht);
    ~SmartPot();

    void begin(esp_sleep_wakeup_cause_t wakeup_cause);
    
    esp_err_t run(QueueHandle_t* queue);
    void handle(int val);

private:
    void _init();

    u8g2_t* display;
    Ds3231* rtc;
    At24cxx* eeprom;
    Dht* dht;
};