/**
 * 
 * 
 */
#pragma once
#include <driver/gpio.h>
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

class MoistureSensor final
{
public:
    MoistureSensor(gpio_num_t data_pin, gpio_num_t power_pin);

    void setup();

    esp_err_t read_raw(int* raw_value);
    esp_err_t read_voltage(int* voltage);

private:
    gpio_num_t _data_pin;
    gpio_num_t _power_pin;
    adc_oneshot_unit_handle_t _adc_handle;
    adc_cali_handle_t _adc_calibration_handle;
};