/**
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <esp_err.h>
#include <esp_log.h>
#include "include/ssd1306_device.h"

/**
 *  @brief SSD1306 device init
 * 
 *  SSD1306 device initialization
 * 
 *  @param[in] dev Device descriptor
 *  @param[in] dev_port Device port
 *  @return
 *      - ESP_OK Success
 *      - ESP_ERR_INVALID_ARG Invalid argument
 */
esp_err_t ssd1306_device_init(ssd1306_device_t* dev, i2c_device_addr_t dev_addr)
{
    dev->i2c_dev.addr = dev_addr;

    return ESP_OK;
}
