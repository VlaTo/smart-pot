/**
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include "crc8.hpp"

#define CRC_TABLE_SIZE      256
#define CRC_POLYNOMIAL      0xD5

Crc8::Crc8()
    : table(NULL)
{
    _init_table();
}

Crc8::~Crc8()
{
    if (table)
    {
        free(table);
        table = NULL;
    }
}

uint8_t Crc8::compute(const uint8_t* in_data, const size_t in_size)
{
    uint8_t crc = 0;

    for (int index = 0; index < in_size; index++)
    {
        crc = (uint8_t)(table[(crc ^ in_data[index]) & 0xFF] ^ (crc >> 8));
    }

    return crc;
}

void Crc8::_init_table()
{
    table = (uint8_t*) malloc(CRC_TABLE_SIZE);
    for (int index = 0; index < CRC_TABLE_SIZE; ++index)
    {
        int temp = index;

        for (int position = 0; position < 8; position++)
        {
            if (0 != (temp & 0x80))
            {
                temp = (temp << 1) ^ CRC_POLYNOMIAL;
                continue;
            }
            
            temp <<= 1;
        }

        table[index] = (uint8_t)temp;
    }
}