/**
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "esp_err.h"
#include "esp_log.h"
#include "i2cdev.hpp"

static const char* TAG = "i2c_dev";

I2cDev::I2cDev(const i2c_port_t dev_port, const i2c_addr_t dev_addr)
    : port(dev_port), addr(dev_addr)
{
}

I2cDev::~I2cDev()
{

}

esp_err_t I2cDev::read_reg(const uint8_t reg, uint8_t* in_data, size_t in_size)
{
    return read_raw(&reg, sizeof(uint8_t), in_data, in_size);
}

esp_err_t I2cDev::read(uint8_t* in_data, size_t in_size)
{
    return read_raw(NULL, 0, in_data, in_size);
}

esp_err_t I2cDev::write_reg(const uint8_t reg, const uint8_t* out_data, size_t out_size)
{
    return write_raw(&reg, sizeof(uint8_t), out_data, out_size);
}

esp_err_t I2cDev::write(const uint8_t* out_data, size_t out_size)
{
    return write_raw(NULL, 0, out_data, out_size);
}

inline i2c_port_t I2cDev::get_port() const
{
    return port;
}

inline i2c_addr_t I2cDev::get_addr() const
{
    return addr;
}

esp_err_t I2cDev::read_raw(const uint8_t* out_data, size_t out_size, uint8_t* in_data, size_t in_size)
{
    if (!in_data || !in_size)
    {
        return ESP_ERR_INVALID_ARG;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    if (out_data && out_size)
    {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(cmd, out_data, out_size, true);
    }

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, in_data, in_size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t result = i2c_master_cmd_begin(port, cmd, pdMS_TO_TICKS(CONFIG_I2C_DEV_TIMEOUT));

    if (ESP_OK != result)
    {
        ESP_LOGE(TAG, "Could not read from device [0x%02x at %d]: %d (%s)", addr, port, result, esp_err_to_name(result));
    }

    i2c_cmd_link_delete(cmd);

    return result;
}

esp_err_t I2cDev::write_raw(const uint8_t* out_reg, size_t out_reg_size, const uint8_t* out_data, size_t out_size)
{
    if (!out_data || !out_size)
    {
        return ESP_ERR_INVALID_ARG;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);

    if (out_reg && out_reg_size)
    {
        i2c_master_write(cmd, out_reg, out_reg_size, true);
    }

    i2c_master_write(cmd, out_data, out_size, true);
    i2c_master_stop(cmd);
    
    esp_err_t result = i2c_master_cmd_begin(port, cmd, pdMS_TO_TICKS(CONFIG_I2C_DEV_TIMEOUT));
    
    if (ESP_OK != result)
    {
        ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)", addr, port, result, esp_err_to_name(result));
    }

    i2c_cmd_link_delete(cmd);
    
    return result;
}