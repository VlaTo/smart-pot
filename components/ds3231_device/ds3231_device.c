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

#define DS3231_ALARM_WDAY   0x40
#define DS3231_ALARM_NOTSET 0x80

#define DS3231_ADDR_TIME    0x00
#define DS3231_ADDR_ALARM1  0x07
#define DS3231_ADDR_ALARM2  0x0b
#define DS3231_ADDR_CONTROL 0x0e
#define DS3231_ADDR_STATUS  0x0f
#define DS3231_ADDR_AGING   0x10
#define DS3231_ADDR_TEMP    0x11

#define DS3231_CTRL_OSCILLATOR    0x80
#define DS3231_CTRL_TEMPCONV      0x20
#define DS3231_CTRL_ALARM_INTS    0x04
#define DS3231_CTRL_ALARM2_INT    0x02
#define DS3231_CTRL_ALARM1_INT    0x01

#define DS3231_12HOUR_FLAG  0x40
#define DS3231_12HOUR_MASK  0x1f
#define DS3231_PM_FLAG      0x20
#define DS3231_MONTH_MASK   0x1f

enum {
    DS3231_SET = 0,
    DS3231_CLEAR,
    DS3231_REPLACE
};

static const char* TAG = "ds3231";

#define _ADDR(dev) &(dev->i2c_dev)

esp_err_t ds3231_device_init(ds3231_device_t* dev, const i2c_device_addr_t dev_addr)
{
    ESP_ERROR_CHECK(i2c_device_init(_ADDR(dev), dev_addr));
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

/* Set/clear bits in a byte register, or replace the byte altogether
 * pass the register address to modify, a byte to replace the existing
 * value with or containing the bits to set/clear and one of
 * DS3231_SET/DS3231_CLEAR/DS3231_REPLACE
 * returns true to indicate success
 */
static esp_err_t ds3231_set_flag(const ds3231_device_t* dev, const uint8_t addr, const uint8_t bits, const uint8_t mode)
{
    uint8_t data;

    /* get status register */
    esp_err_t res = i2c_device_read_reg(_ADDR(dev), addr, &data, sizeof(uint8_t));

    if (res != ESP_OK) {
        return res;
    }

    /* clear the flag */
    if (mode == DS3231_REPLACE) {
        data = bits;
    }
    else if (mode == DS3231_SET) {
        data |= bits;
    }
    else {
        data &= ~bits;
    }

    return i2c_device_write_reg(_ADDR(dev), addr, &data, 1);
}

/* Get a byte containing just the requested bits
 * pass the register address to read, a mask to apply to the register and
 * an uint* for the output
 * you can test this value directly as true/false for specific bit mask
 * of use a mask of 0xff to just return the whole register byte
 * returns true to indicate success
 */
static esp_err_t ds3231_get_flag(const ds3231_device_t* dev, const uint8_t addr, const uint8_t mask, uint8_t *flag)
{
    uint8_t data;

    /* get register */
    esp_err_t res = i2c_device_read_reg(_ADDR(dev), addr, &data, 1);

    if (res != ESP_OK) {
        return res;
    }

    /* return only requested flag */
    *flag = (data & mask);

    return ESP_OK;
}

esp_err_t ds3231_get_time(const ds3231_device_t* dev, struct tm* time)
{
    if (!dev || !time) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[7];

    I2C_DEVICE_TAKE(_ADDR(dev));
    /* read time */
    ESP_ERROR_CHECK(i2c_device_read_reg(_ADDR(dev), DS3231_ADDR_TIME, data, sizeof(data)));
    I2C_DEVICE_GIVE(_ADDR(dev));

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

esp_err_t ds3231_set_time(const ds3231_device_t *dev, const struct tm *time)
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

    esp_err_t result;

    I2C_DEVICE_TAKE(_ADDR(dev));
    result = i2c_device_write_reg(_ADDR(dev), DS3231_ADDR_TIME, data, sizeof(data));
    I2C_DEVICE_GIVE(_ADDR(dev));

    return result;
}

esp_err_t ds3231_set_alarm(
    const ds3231_device_t* dev,
    const ds3231_alarm_t alarms,
    const struct tm* time1,
    const ds3231_alarm1_rate_t option1,
    const struct tm* time2,
    const ds3231_alarm2_rate_t option2)
{
    if (!dev) {
        return ESP_ERR_INVALID_ARG;
    }

    int i = 0;
    uint8_t data[7];

    /* alarm 1 data */
    if (DS3231_ALARM_2 != alarms)
    {
        if (!time1) {
            return ESP_ERR_INVALID_ARG;
        }

        data[i++] = (DS3231_ALARM1_MATCH_SEC <= option1 ? dec2bcd(time1->tm_sec) : DS3231_ALARM_NOTSET);
        data[i++] = (option1 >= DS3231_ALARM1_MATCH_SECMIN ? dec2bcd(time1->tm_min) : DS3231_ALARM_NOTSET);
        data[i++] = (option1 >= DS3231_ALARM1_MATCH_SECMINHOUR ? dec2bcd(time1->tm_hour) : DS3231_ALARM_NOTSET);
        data[i++] = (DS3231_ALARM1_MATCH_SECMINHOURDAY == option1
            ? (dec2bcd(time1->tm_wday + 1) & DS3231_ALARM_WDAY)
            : (DS3231_ALARM1_MATCH_SECMINHOURDATE == option1 ? dec2bcd(time1->tm_mday) : DS3231_ALARM_NOTSET));
    }

    /* alarm 2 data */
    if (DS3231_ALARM_1 != alarms)
    {
        if (!time2) {
            return ESP_ERR_INVALID_ARG;
        }

        data[i++] = (option2 >= DS3231_ALARM2_MATCH_MIN ? dec2bcd(time2->tm_min) : DS3231_ALARM_NOTSET);
        data[i++] = (option2 >= DS3231_ALARM2_MATCH_MINHOUR ? dec2bcd(time2->tm_hour) : DS3231_ALARM_NOTSET);
        data[i++] = (DS3231_ALARM2_MATCH_MINHOURDAY == option2
            ? (dec2bcd(time2->tm_wday + 1) & DS3231_ALARM_WDAY)
            : (DS3231_ALARM2_MATCH_MINHOURDATE == option2 ? dec2bcd(time2->tm_mday) : DS3231_ALARM_NOTSET));
    }

    I2C_DEVICE_TAKE(_ADDR(dev));
    ESP_ERROR_CHECK(i2c_device_write_reg(_ADDR(dev), (DS3231_ALARM_2 == alarms ? DS3231_ADDR_ALARM2 : DS3231_ADDR_ALARM1), data, i));
    I2C_DEVICE_GIVE(_ADDR(dev));

    return ESP_OK;
}

esp_err_t ds3231_clear_alarm_flags(const ds3231_device_t* dev, const ds3231_alarm_t alarms)
{
    if (!dev) {
        return ESP_ERR_INVALID_ARG;
    }

    I2C_DEVICE_TAKE(_ADDR(dev));
    ESP_ERROR_CHECK(ds3231_set_flag(dev, DS3231_ADDR_STATUS, alarms, DS3231_CLEAR));
    I2C_DEVICE_GIVE(_ADDR(dev));

    return ESP_OK;
}

esp_err_t ds3231_get_alarm_flags(const ds3231_device_t* dev, ds3231_alarm_t* alarms)
{
    if (!dev || !alarms) {
        return ESP_ERR_INVALID_ARG;
    }

    I2C_DEVICE_TAKE(_ADDR(dev));
    ESP_ERROR_CHECK(ds3231_get_flag(dev, DS3231_ADDR_STATUS, DS3231_ALARM_BOTH, (uint8_t *)alarms));
    I2C_DEVICE_GIVE(_ADDR(dev));

    return ESP_OK;
}

esp_err_t ds3231_enable_alarm_ints(const ds3231_device_t* dev, const ds3231_alarm_t alarms)
{
    if (!dev) {
        return ESP_ERR_INVALID_ARG;
    }

    I2C_DEVICE_TAKE(_ADDR(dev));
    ESP_ERROR_CHECK(ds3231_set_flag(dev, DS3231_ADDR_CONTROL, DS3231_CTRL_ALARM_INTS | alarms, DS3231_SET));
    I2C_DEVICE_GIVE(_ADDR(dev));

    return ESP_OK;
}

esp_err_t ds3231_disable_alarm_ints(const ds3231_device_t* dev, const ds3231_alarm_t alarms)
{
    if (!dev) {
        return ESP_ERR_INVALID_ARG;
    }

    /* Just disable specific alarm(s) requested
     * does not disable alarm interrupts generally (which would enable the squarewave)
     */
    I2C_DEVICE_TAKE(_ADDR(dev));
    ESP_ERROR_CHECK(ds3231_set_flag(dev, DS3231_ADDR_CONTROL, alarms, DS3231_CLEAR));
    I2C_DEVICE_GIVE(_ADDR(dev));

    return ESP_OK;
}

esp_err_t ds3231_enable_squarewave(const ds3231_device_t* dev)
{
    if (!dev) {
        return ESP_ERR_INVALID_ARG;
    }

    I2C_DEVICE_TAKE(_ADDR(dev));
    ESP_ERROR_CHECK(ds3231_set_flag(dev, DS3231_ADDR_CONTROL, DS3231_CTRL_ALARM_INTS, DS3231_CLEAR));
    I2C_DEVICE_GIVE(_ADDR(dev));

    return ESP_OK;
}

esp_err_t ds3231_disable_squarewave(const ds3231_device_t* dev)
{
    if (!dev) {
        return ESP_ERR_INVALID_ARG;
    }

    I2C_DEVICE_TAKE(_ADDR(dev));
    ESP_ERROR_CHECK(ds3231_set_flag(dev, DS3231_ADDR_CONTROL, DS3231_CTRL_ALARM_INTS, DS3231_SET));
    I2C_DEVICE_GIVE(_ADDR(dev));

    return ESP_OK;
}

esp_err_t ds3231_set_squarewave_freq(const ds3231_device_t* dev, const ds3231_sqwave_freq_t freq)
{
    if (!dev) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t flag = 0;

    I2C_DEVICE_TAKE(_ADDR(dev));
    ESP_ERROR_CHECK(ds3231_get_flag(dev, DS3231_ADDR_CONTROL, 0xFF, &flag));
    flag &= ~DS3231_SQWAVE_8192HZ;
    flag |= freq;
    ESP_ERROR_CHECK(ds3231_set_flag(dev, DS3231_ADDR_CONTROL, flag, DS3231_REPLACE));
    I2C_DEVICE_GIVE(_ADDR(dev));

    return ESP_OK;
}

esp_err_t ds3231_get_squarewave_freq(const ds3231_device_t* dev, ds3231_sqwave_freq_t* freq)
{
    if (!dev) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t flag = 0;

    I2C_DEVICE_TAKE(_ADDR(dev));
    ESP_ERROR_CHECK(ds3231_get_flag(dev, DS3231_ADDR_CONTROL, 0xFF, &flag));
    I2C_DEVICE_GIVE(_ADDR(dev));

    flag &= DS3231_SQWAVE_8192HZ;
    *freq = (ds3231_sqwave_freq_t) flag;

    return ESP_OK;
}

esp_err_t ds3231_get_raw_temp(const ds3231_device_t* dev, int16_t* temp)
{
    if (!dev || !temp) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[2];

    I2C_DEVICE_TAKE(_ADDR(dev));
    ESP_ERROR_CHECK(i2c_device_read_reg(_ADDR(dev), DS3231_ADDR_TEMP, data, sizeof(data)));
    I2C_DEVICE_GIVE(_ADDR(dev));

    *temp = (int16_t)(int8_t)data[0] << 2 | data[1] >> 6;

    return ESP_OK;
}

esp_err_t ds3231_get_temp_integer(const ds3231_device_t* dev, int8_t* temp)
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

esp_err_t ds3231_get_temp_float(const ds3231_device_t* dev, float* temp)
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

esp_err_t ds3231_set_aging_offset(const ds3231_device_t* dev, const int8_t age)
{
    if (!dev) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t age_u8 = (uint8_t) age;

    I2C_DEVICE_TAKE(_ADDR(dev));
    ESP_ERROR_CHECK(i2c_device_write_reg(_ADDR(dev), DS3231_ADDR_AGING, &age_u8, sizeof(uint8_t)));
    /**
     * To see the effects of the aging register on the 32kHz output
     * frequency immediately, a manual conversion should be started
     * after each aging register change.
     */
    ESP_ERROR_CHECK(ds3231_set_flag(dev, DS3231_ADDR_CONTROL, DS3231_CTRL_TEMPCONV, DS3231_SET));
    I2C_DEVICE_GIVE(_ADDR(dev));

    return ESP_OK;
}

esp_err_t ds3231_get_aging_offset(const ds3231_device_t* dev, int8_t *age)
{
    if (!dev || !age) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t age_u8;

    I2C_DEVICE_TAKE(_ADDR(dev));
    ESP_ERROR_CHECK(i2c_device_read_reg(_ADDR(dev), DS3231_ADDR_AGING, &age_u8, sizeof(uint8_t)));
    I2C_DEVICE_GIVE(_ADDR(dev));

    *age = (int8_t) age_u8;

    return ESP_OK;
}