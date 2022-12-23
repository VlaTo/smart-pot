/**
 * 
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include "esp_err.h"
#include "esp_log.h"
#include "menu_item.hpp"
#include "menu.hpp"

/**
 * 
 */
Menu::Menu(MenuItem** menu_items, const int16_t menu_items_count)
    : _menu_items(menu_items), _menu_items_count(menu_items_count), _top_item_index(0), _selected_item_index(-1)
{
}

Menu::~Menu()
{    
}

/*constexpr int16_t Menu::get_count() const
{
    return 0;
}*/

MenuItem* Menu::operator[](int16_t index)
{
    return _menu_items[index];
}

/*esp_err_t Menu::draw(u8g2* u8g2) const
{
    u8g2_uint_t height = u8g2_GetDisplayHeight(u8g2);
    u8g2_uint_t width = u8g2_GetDisplayWidth(u8g2);

    for (int16_t index = _top_item_index; _menu_items_count > index; index++)
    {
        int16_t 
    }
    return ESP_OK;
}*/