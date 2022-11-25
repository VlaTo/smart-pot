/**
 * 
 * 
 */

#ifndef __DS3231_DEVICE_H__
#define __DS3231_DEVICE_H__

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/i2c.h>
#include "i2c_bus.h"

#define DEFAULT_DS3231_ADDR    ((i2c_device_addr_t)0x68)

typedef struct {
    i2c_device_t i2c_dev;    
} ds3231_device_t;

/**
 *  @brief DS3231 device init
 * 
 *  DS3231 device initialization
 * 
 *  @param[in] dev Device descriptor
 *  @param[in] dev_port Device port
 * 
 *  @return
 *      - ESP_OK Success
 *      - ESP_ERR_INVALID_ARG Invalid argument
 */
esp_err_t ds3231_device_init(ds3231_device_t* dev, i2c_device_addr_t dev_addr);

/**
 *  @brief Gets time
 * 
 *  Gets device 
 */
esp_err_t ds3231_get_time(ds3231_device_t* dev, struct tm* time);

/**
 * @brief Set the time on the RTC
 *
 * Timezone agnostic, pass whatever you like.
 * I suggest using GMT and applying timezone and DST when read back.
 *
 * @return ESP_OK to indicate success
 */
esp_err_t ds3231_set_time(ds3231_device_t *dev, struct tm *time);

/**
 * @brief Get the raw temperature value
 *
 * **Supported only by DS3231**
 *
 * @param dev Device descriptor
 * @param[out] temp Raw temperature value
 * @return ESP_OK to indicate success
 */
esp_err_t ds3231_get_raw_temp(ds3231_device_t* dev, int16_t* temp);

esp_err_t ds3231_get_temp_integer(ds3231_device_t* dev, int8_t* temp);

esp_err_t ds3231_get_temp_float(ds3231_device_t* dev, float* temp);

#endif /* __DS3231_DEVICE_H__ */