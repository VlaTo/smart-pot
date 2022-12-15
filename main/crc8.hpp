/**
 * 
 * 
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>

class Crc8 final
{
public:
    Crc8();
    ~Crc8();

    uint8_t compute(const uint8_t* in_data, const size_t in_size);

private:
    void _init_table();

    uint8_t* table;
};