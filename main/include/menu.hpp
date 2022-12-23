/**
 * 
 * 
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "menu_item.hpp"
#include "u8g2.h"

/*
 */
class Menu final
{
public:
    Menu(MenuItem** menu_items, const int16_t menu_items_count);
    ~Menu();

    constexpr int16_t get_count() const
    {
        return _menu_items_count;
    }

    //esp_err_t draw(u8g2* u8g2) const;

    MenuItem* operator[](int16_t index);

private:
    MenuItem** _menu_items;
    int16_t _menu_items_count;
    int16_t _top_item_index;
    int16_t _selected_item_index;
};