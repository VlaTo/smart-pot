/**
 * 
 * 
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <bitset>

/*
 */
typedef void (*menu_item_cb)();

/*
 */
class MenuItem final
{
public:
    MenuItem(const char* title, menu_item_cb cb);
    MenuItem(const char* title, MenuItem* sub_menu);
    ~MenuItem();

    bool has_submenu() const;
    inline const char* get_title() const
    {
        return _title;
    }

private:

    std::bitset<3> _flags;
    const char* _title;
    union
    {
        menu_item_cb _cb;
        MenuItem* _sub_menu;
    };
};
