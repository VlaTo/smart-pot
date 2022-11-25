/**
 * 
 * 
 */

#ifndef __I2C_BUS_H__
#define __I2C_BUS_H__

#include <stdio.h>
#include <string.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#define I2C_MASTER_SCL_IO 22                /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 21                /*!< gpio number for I2C master data  */
#define I2C_MASTER_FREQ_HZ 400000           /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0         /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0         /*!< I2C master doesn't need buffer */

typedef uint8_t i2c_device_addr_t;

typedef struct {
    i2c_device_addr_t addr;
} i2c_device_t;

/**
 *  @brief Initialize I2C bus
 * 
 *  Initializes I2C bus in master mode
 * 
 *  @param[in] mode Bus mode
 *  @param[in] port_num I2C port numer
 */
 esp_err_t i2c_master_init(const i2c_port_t port_num);

/**
 *  @brief Initialize device
 * 
 *  Initialize I2C device
 * 
 *  @param[in] dev Device descriptor
 *  @param[in] dev_addr Device address
 */
esp_err_t i2c_device_init(i2c_device_t* dev, const i2c_device_addr_t dev_addr);

esp_err_t i2c_device_write(const i2c_device_t* dev, const void* data, const size_t data_size);

esp_err_t i2c_device_write_reg(const i2c_device_t* dev, uint8_t reg, const void* data, const size_t data_size);

esp_err_t i2c_device_read(const i2c_device_t* dev, void* data, const size_t data_size);

esp_err_t i2c_device_read_reg(const i2c_device_t* dev, uint8_t reg, void* data, const size_t data_size);

#endif