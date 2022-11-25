/**
 * 
 * 
 */

#ifndef __SSD1306_DEVICE_H__
#define __SSD1306_DEVICE_H__

#include <stdio.h>
#include <string.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/i2c.h>
#include "i2c_bus.h"

//#define I2C_MASTER_SCL_IO 22                /*!< gpio number for I2C master clock */
#define DEFAULT_SSD1306_ADDR    ((i2c_device_addr_t)0x3C)

typedef struct {
    i2c_device_t i2c_dev;    
} ssd1306_device_t;

/**
 *  @brief Initialize I2C bus
 * 
 *  Initializes I2C bus in master mode
 * 
 *  @param[in] mode Bus mode
 *  @param[in] port_num I2C port numer
 */
esp_err_t ssd1306_device_init(ssd1306_device_t* dev, i2c_device_addr_t dev_addr);

#endif /* __SSD1306_DEVICE_H__ */