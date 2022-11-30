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

typedef enum {
    DS3231_ALARM_NONE = 0,//!< No alarms
    DS3231_ALARM_1,       //!< First alarm
    DS3231_ALARM_2,       //!< Second alarm
    DS3231_ALARM_BOTH     //!< Both alarms
} ds3231_alarm_t;

typedef enum {
    DS3231_ALARM1_EVERY_SECOND = 0,
    DS3231_ALARM1_MATCH_SEC,
    DS3231_ALARM1_MATCH_SECMIN,
    DS3231_ALARM1_MATCH_SECMINHOUR,
    DS3231_ALARM1_MATCH_SECMINHOURDAY,
    DS3231_ALARM1_MATCH_SECMINHOURDATE
} ds3231_alarm1_rate_t;

typedef enum {
    DS3231_ALARM2_EVERY_MIN = 0,
    DS3231_ALARM2_MATCH_MIN,
    DS3231_ALARM2_MATCH_MINHOUR,
    DS3231_ALARM2_MATCH_MINHOURDAY,
    DS3231_ALARM2_MATCH_MINHOURDATE
} ds3231_alarm2_rate_t;

typedef enum {
    DS3231_SQWAVE_1HZ    = 0x00,
    DS3231_SQWAVE_1024HZ = 0x08,
    DS3231_SQWAVE_4096HZ = 0x10,
    DS3231_SQWAVE_8192HZ = 0x18
} ds3231_sqwave_freq_t;

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
esp_err_t ds3231_device_init(ds3231_device_t* dev, const i2c_device_addr_t dev_addr);

/**
 *  @brief Gets time
 * 
 *  Gets device 
 */
esp_err_t ds3231_get_time(const ds3231_device_t* dev, struct tm* time);

/**
 * @brief Set the time on the RTC
 *
 * Timezone agnostic, pass whatever you like.
 * I suggest using GMT and applying timezone and DST when read back.
 *
 * @return ESP_OK to indicate success
 */
esp_err_t ds3231_set_time(const ds3231_device_t *dev, const struct tm* time);

/**
 * @brief Set alarms
 *
 * `alarm1` works with seconds, minutes, hours and day of week/month, or fires every second.
 * `alarm2` works with minutes, hours and day of week/month, or fires every minute.
 *
 * Not all combinations are supported, see `DS3231_ALARM1_*` and `DS3231_ALARM2_*` defines
 * for valid options you only need to populate the fields you are using in the `tm` struct,
 * and you can set both alarms at the same time (pass `DS3231_ALARM_1`/`DS3231_ALARM_2`/`DS3231_ALARM_BOTH`).
 *
 * If only setting one alarm just pass 0 for `tm` struct and `option` field for the other alarm.
 * If using ::DS3231_ALARM1_EVERY_SECOND/::DS3231_ALARM2_EVERY_MIN you can pass 0 for `tm` struct.
 *
 * If you want to enable interrupts for the alarms you need to do that separately.
 *
 * @return ESP_OK to indicate success
 */
esp_err_t ds3231_set_alarm(
    const ds3231_device_t* dev,
    const ds3231_alarm_t alarms,
    const struct tm* time1,
    const ds3231_alarm1_rate_t option1,
    const struct tm* time2,
    const ds3231_alarm2_rate_t option2
);

/**
 * @brief Clear alarm past flag(s)
 *
 * Pass `DS3231_ALARM_1`/`DS3231_ALARM_2`/`DS3231_ALARM_BOTH`
 *
 * @param dev Device descriptor
 * @param alarms Alarms
 * @return ESP_OK to indicate success
 */
esp_err_t ds3231_clear_alarm_flags(const ds3231_device_t* dev, const ds3231_alarm_t alarms);

/**
 * @brief Check which alarm(s) have past
 *
 * Sets alarms to `DS3231_ALARM_NONE`/`DS3231_ALARM_1`/`DS3231_ALARM_2`/`DS3231_ALARM_BOTH`
 *
 * @param dev Device descriptor
 * @param[out] alarms Alarms
 * @return ESP_OK to indicate success
 */
esp_err_t ds3231_get_alarm_flags(const ds3231_device_t* dev, ds3231_alarm_t* alarms);

/**
 * @brief enable alarm interrupts (and disables squarewave)
 *
 * Pass `DS3231_ALARM_1`/`DS3231_ALARM_2`/`DS3231_ALARM_BOTH`.
 *
 * If you set only one alarm the status of the other is not changed
 * you must also clear any alarm past flag(s) for alarms with
 * interrupt enabled, else it will trigger immediately.
 *
 * @param dev Device descriptor
 * @param alarms Alarms
 * @return ESP_OK to indicate success
 */
esp_err_t ds3231_enable_alarm_ints(const ds3231_device_t* dev, const ds3231_alarm_t alarms);

/**
 * @brief Disable alarm interrupts
 *
 * Does not (re-)enable squarewave
 *
 * @param dev Device descriptor
 * @param alarms Alarm
 * @return ESP_OK to indicate success
 */
esp_err_t ds3231_disable_alarm_ints(const ds3231_device_t* dev, const ds3231_alarm_t alarms);

/**
 * @brief Enable the squarewave output
 *
 * Disables alarm interrupt functionality.
 *
 * @param dev Device descriptor
 * @return ESP_OK to indicate success
 */
esp_err_t ds3231_enable_squarewave(const ds3231_device_t* dev);

/**
 * @brief Disable the squarewave output
 *
 * Which re-enables alarm interrupts, but individual alarm interrupts also
 * need to be enabled, if not already, before they will trigger.
 *
 * @param dev Device descriptor
 * @return ESP_OK to indicate success
 */
esp_err_t ds3231_disable_squarewave(const ds3231_device_t* dev);

/**
 * @brief Set the frequency of the squarewave output
 *
 * Does not enable squarewave output.
 *
 * @param dev Device descriptor
 * @param freq Squarewave frequency
 * @return ESP_OK to indicate success
 */
esp_err_t ds3231_set_squarewave_freq(const ds3231_device_t* dev, const ds3231_sqwave_freq_t freq);

/**
 * @brief Get the frequency of the squarewave output
 *
 * Does not enable squarewave output.
 *
 * @param dev Device descriptor
 * @param freq Squarewave frequency to store the output
 * @return ESP_OK to indicate success
 */
esp_err_t ds3231_get_squarewave_freq(const ds3231_device_t* dev, ds3231_sqwave_freq_t* freq);

/**
 * @brief Get the raw temperature value
 *
 * **Supported only by DS3231**
 *
 * @param dev Device descriptor
 * @param[out] temp Raw temperature value
 * 
 * @return ESP_OK to indicate success
 */
esp_err_t ds3231_get_raw_temp(const ds3231_device_t* dev, int16_t* temp);

/**
 * @brief Get the temperature as an integer
 *
 * **Supported only by DS3231**
 *
 * @param dev Device descriptor
 * @param[out] temp Temperature, degrees Celsius
 * @return ESP_OK to indicate success
 */
esp_err_t ds3231_get_temp_integer(const ds3231_device_t* dev, int8_t* temp);

/**
 * @brief Get the temperature as a float
 *
 * **Supported only by DS3231**
 *
 * @param dev Device descriptor
 * @param[out] temp Temperature, degrees Celsius
 * @return ESP_OK to indicate success
 */
esp_err_t ds3231_get_temp_float(const ds3231_device_t* dev, float* temp);

/**
 * @brief Set the aging offset register to a new value.
 *
 * Positive aging values add capacitance to the array,
 * slowing the oscillator frequency. Negative values remove
 * capacitance from the array, increasing the oscillator
 * frequency.
 *
 * **Supported only by DS3231**
 *
 * @param dev Device descriptor
 * @param age Aging offset (in range [-128, 127]) to be set
 * 
 * @return ESP_OK to indicate success
 */
esp_err_t ds3231_set_aging_offset(const ds3231_device_t* dev, const int8_t age);

/**
 * @brief Get the aging offset register.
 *
 * **Supported only by DS3231**
 *
 * @param dev Device descriptor
 * @param[out] age Aging offset in range [-128, 127]
 * 
 * @return ESP_OK to indicate success
 */
esp_err_t ds3231_get_aging_offset(const ds3231_device_t* dev, int8_t *age);

#endif /* __DS3231_DEVICE_H__ */