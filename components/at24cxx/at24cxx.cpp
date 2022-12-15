/**
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "at24cxx.hpp"

At24cxx::At24cxx(const i2c_port_t i2c_port, const i2c_addr_t dev_addr, at24_mem_size_t mem_size)
    : I2cDev(i2c_port, dev_addr), size(mem_size)
{
}

esp_err_t At24cxx::read(const uint16_t offset, uint8_t* out_data, const size_t out_size)
{
    uint8_t data[sizeof(offset)];

    data[0] = (uint8_t)(offset >> 8);       // page num
    data[1] = (uint8_t)(offset & 0xFF);     // offset in page

    ESP_ERROR_CHECK(read_raw(data, sizeof(data), out_data, out_size));

    return ESP_OK;
}

esp_err_t At24cxx::write(const uint16_t offset, const uint8_t* in_data, const size_t in_size)
{
    uint8_t data[sizeof(offset)];

    data[0] = (uint8_t)(offset >> 8);
    data[1] = (uint8_t)(offset & 0xFF);

    ESP_ERROR_CHECK(write_raw(data, sizeof(data), in_data, in_size));

    return ESP_OK;
}