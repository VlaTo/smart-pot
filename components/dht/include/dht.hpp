/**
 * 
 * 
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <driver/gpio.h>
#include "esp_err.h"

typedef enum
{
    DHT_TYPE_DHT11 = 0,
    DHT_TYPE_AM2301,
    DHT_TYPE_SI7021
} dht_sensor_type_t;

class Dht final
{
public:
    Dht(gpio_num_t pin, dht_sensor_type_t sensor_type);

    /**
     * @brief Read integer data from sensor on specified pin
     *
     * Humidity and temperature are returned as integers.
     * For example: humidity=625 is 62.5 %, temperature=244 is 24.4 degrees Celsius
     *
     * @param sensor_type DHT11 or DHT22
     * @param pin GPIO pin connected to sensor OUT
     * @param[out] humidity Humidity, percents * 10, nullable
     * @param[out] temperature Temperature, degrees Celsius * 10, nullable
     * @return `ESP_OK` on success
     */
    esp_err_t read_data(int16_t* humidity, int16_t* temperature);
    /**
     * @brief Read float data from sensor on specified pin
     *
     * Humidity and temperature are returned as floats.
     *
     * @param sensor_type DHT11 or DHT22
     * @param pin GPIO pin connected to sensor OUT
     * @param[out] humidity Humidity, percents, nullable
     * @param[out] temperature Temperature, degrees Celsius, nullable
     * @return `ESP_OK` on success
     */
    esp_err_t read_float_data(float* humidity, float* temperature);

private:
    esp_err_t await_pin_state(uint32_t timeout, int expected_pin_state, uint32_t* duration);
    inline esp_err_t fetch_data(uint8_t* data);
    inline int16_t convert_data(uint8_t msb, uint8_t lsb);

    gpio_num_t _pin;
    dht_sensor_type_t _sensor_type;
    TickType_t _ticks;
};