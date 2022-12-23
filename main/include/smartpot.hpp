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

/**
 * 
 */
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