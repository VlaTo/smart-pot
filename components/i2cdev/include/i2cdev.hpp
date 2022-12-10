/**
 * 
 * 
*/

#pragma once

#include <stdio.h>
#include "esp_err.h"
#include "driver/i2c.h"

typedef uint8_t i2c_addr_t;

class I2cDev
{
public:
    i2c_port_t get_port() const;
    i2c_addr_t get_addr() const;

protected:
    I2cDev(const i2c_port_t dev_port, const i2c_addr_t dev_addr);
    ~I2cDev();

    esp_err_t read_reg(const uint8_t reg, uint8_t* in_data, size_t in_size);
    esp_err_t read(uint8_t* in_data, size_t in_size);
    esp_err_t write_reg(const uint8_t reg, const uint8_t* out_data, size_t out_size);
    esp_err_t write(const uint8_t* out_data, size_t out_size);

    esp_err_t read_raw(const uint8_t* out_data, size_t out_size, uint8_t* in_data, size_t in_size);
    esp_err_t write_raw(const uint8_t* out_reg, size_t out_reg_size, const uint8_t* out_data, size_t out_size);

    i2c_port_t port;
    i2c_addr_t addr;
};