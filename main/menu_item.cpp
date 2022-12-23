/**
 * 
 * 
 */
#include <stdio.h>
#include <stdlib.h>
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "freertos/semphr.h"
//#include "freertos/event_groups.h"
#include "esp_err.h"
#include "esp_log.h"
//#include "esp_sleep.h"
//#include "esp_event.h"
//#include "ds3231.hpp"
//#include "at24cxx.hpp"
//#include "smartpot.hpp"
//#include "encoder.h"
#include "menu_item.hpp"

#define BITS_HAS_SUBMENU        0

/**
 * 
 */
MenuItem::MenuItem(const char* title, menu_item_cb cb)
    : _flags(), _title(title), _cb(cb)
{
}

MenuItem::MenuItem(const char* title, MenuItem* sub_menu)
    : _flags(), _title(title), _sub_menu(sub_menu)
{
    _flags.set(BITS_HAS_SUBMENU);
}

MenuItem::~MenuItem()
{
}

bool MenuItem::has_submenu() const
{
    return _flags.test(BITS_HAS_SUBMENU);
}