/**
 * 
 * 
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "i2cdev.hpp"

#define AT24CXX_ADDR    ((i2c_addr_t)0x57)

typedef enum
{
    CHIP_8x4096 = 32,
    CHIP_16x4096 = 64
} at24_mem_size_t;

class At24cxx final : public I2cDev
{
public:
    At24cxx(const i2c_port_t dev_port, const i2c_addr_t dev_addr, at24_mem_size_t mem_size);

    esp_err_t read(const uint16_t offset, uint8_t* out_data, const size_t out_size);
    esp_err_t write(const uint16_t offset, const uint8_t* in_data, const size_t in_size);

private:
    at24_mem_size_t size;
};