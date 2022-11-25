/**
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <esp_err.h>
#include <esp_log.h>
#include "i2c_bus.h"
#include "ds3231_device.h"

#define DS3231_ADDR_TIME    0x00
#define DS3231_ADDR_ALARM1  0x07
#define DS3231_ADDR_ALARM2  0x0b
#define DS3231_ADDR_CONTROL 0x0e
#define DS3231_ADDR_STATUS  0x0f
#define DS3231_ADDR_AGING   0x10
#define DS3231_ADDR_TEMP    0x11

#define DS3231_12HOUR_FLAG  0x40
#define DS3231_12HOUR_MASK  0x1f
#define DS3231_PM_FLAG      0x20
#define DS3231_MONTH_MASK   0x1f

static const char* TAG = "ds3231";

esp_err_t ds3231_device_init(ds3231_device_t* dev, i2c_device_addr_t dev_addr)
{
    ESP_ERROR_CHECK(i2c_device_init(&(dev->i2c_dev), dev_addr));
    return ESP_OK;
}

static uint8_t bcd2dec(uint8_t value)
{
    return (value >> 4) * 10 + (value & 0x0F);
}

static uint8_t dec2bcd(uint8_t value)
{
    return ((value / 10) << 4) + (value % 10);
}

esp_err_t ds3231_get_time(ds3231_device_t* dev, struct tm* time)
{
    if (!dev || !time) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[7];

    /* read time */
    ESP_ERROR_CHECK(i2c_device_read_reg(&(dev->i2c_dev), DS3231_ADDR_TIME, data, sizeof(data)));

    /* convert to unix time structure */
    time->tm_sec = bcd2dec(data[0]);
    time->tm_min = bcd2dec(data[1]);
    if (data[2] & DS3231_12HOUR_FLAG) {
        /* 12H */
        time->tm_hour = bcd2dec(data[2] & DS3231_12HOUR_MASK) - 1;
        /* AM/PM? */
        if (data[2] & DS3231_PM_FLAG) {
            time->tm_hour += 12;
        }
    }
    else {
        time->tm_hour = bcd2dec(data[2]); /* 24H */
    }
    time->tm_wday = bcd2dec(data[3]) - 1;
    time->tm_mday = bcd2dec(data[4]);
    time->tm_mon  = bcd2dec(data[5] & DS3231_MONTH_MASK) - 1;
    time->tm_year = bcd2dec(data[6]) + 100;
    time->tm_isdst = 0;

    // apply a time zone (if you are not using localtime on the rtc or you want to check/apply DST)
    //applyTZ(time);

    return ESP_OK;
}

esp_err_t ds3231_set_time(ds3231_device_t *dev, struct tm *time)
{
    if (!dev || !time) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[7];

    /* time/date data */
    data[0] = dec2bcd(time->tm_sec);
    data[1] = dec2bcd(time->tm_min);
    data[2] = dec2bcd(time->tm_hour);
    /* The week data must be in the range 1 to 7, and to keep the start on the
     * same day as for tm_wday have it start at 1 on Sunday. */
    data[3] = dec2bcd(time->tm_wday + 1);
    data[4] = dec2bcd(time->tm_mday);
    data[5] = dec2bcd(time->tm_mon + 1);
    data[6] = dec2bcd(time->tm_year - 100);

    return i2c_device_write_reg(&(dev->i2c_dev), DS3231_ADDR_TIME, data, sizeof(data));
}

esp_err_t ds3231_get_raw_temp(ds3231_device_t* dev, int16_t* temp)
{
    if (!dev || !temp) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[2];

    ESP_ERROR_CHECK(i2c_device_read_reg(&(dev->i2c_dev), DS3231_ADDR_TEMP, data, sizeof(data)));

    *temp = (int16_t)(int8_t)data[0] << 2 | data[1] >> 6;

    return ESP_OK;
}

esp_err_t ds3231_get_temp_integer(ds3231_device_t* dev, int8_t* temp)
{
    if (!dev || !temp) {
        return ESP_ERR_INVALID_ARG;
    }

    int16_t t_int;

    esp_err_t res = ds3231_get_raw_temp(dev, &t_int);

    if (ESP_OK == res) {
        *temp = t_int >> 2;
    }

    return res;
}

esp_err_t ds3231_get_temp_float(ds3231_device_t* dev, float* temp)
{
    if (!dev || !temp) {
        return ESP_ERR_INVALID_ARG;
    }

    int16_t t_int;

    esp_err_t res = ds3231_get_raw_temp(dev, &t_int);

    if (ESP_OK == res) {
        *temp = t_int * 0.25;
    }

    return res;
}