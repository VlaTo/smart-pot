/**
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "esp_err.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "moisture_sensor.hpp"

static const char* TAG = "moisture-sensor";

MoistureSensor::MoistureSensor(gpio_num_t data_pin, gpio_num_t power_pin)
    : _data_pin(data_pin), _power_pin(power_pin)
{
}

void MoistureSensor::setup()
{
    gpio_set_direction(_power_pin, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(_power_pin, GPIO_FLOATING);
    
    //gpio_set_level(MS_POWER_PIN, 1);

    //adc_oneshot_unit_handle_t adc2_handle;
    adc_oneshot_unit_init_cfg_t adc_config = {
        .unit_id = ADC_UNIT_2,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_config, &_adc_handle));

    adc_oneshot_chan_cfg_t adc_channel_config = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(_adc_handle, ADC_CHANNEL_3, &adc_channel_config));

    adc_cali_line_fitting_config_t adc_calibration_config = {
            .unit_id = ADC_UNIT_2,
            .atten = ADC_ATTEN_DB_11,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
            .default_vref = 128lu
        };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&adc_calibration_config, &_adc_calibration_handle));

    //int m_raw_value, m_voltage;
    //ESP_ERROR_CHECK(adc_oneshot_read(_adc_handle, ADC_CHANNEL_3, &m_raw_value));
    //ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_2, ADC_CHANNEL_3, m_raw_value);
    //ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc2_calibration_handle, m_raw_value, &m_voltage));
    //ESP_LOGI(TAG, "ADC%d Channel[%d] Calibrated Voltage: %d mV", ADC_UNIT_2, ADC_CHANNEL_3, m_voltage);
}

esp_err_t MoistureSensor::read_raw(int* raw_value)
{
    esp_err_t result = adc_oneshot_read(_adc_handle, ADC_CHANNEL_3, raw_value);
    if (ESP_OK == result)
    {
        ESP_LOGD(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_2, ADC_CHANNEL_3, *raw_value);        
    }

    return result;
}

esp_err_t MoistureSensor::read_voltage(int* voltage)
{
    int raw_value;
    esp_err_t result = read_raw(&raw_value);

    if (ESP_OK == result)
    {
        result = adc_cali_raw_to_voltage(_adc_calibration_handle, raw_value, voltage);
        if (ESP_OK == result)
        {
            ESP_LOGD(TAG, "ADC%d Channel[%d] Calibrated Voltage: %d mV", ADC_UNIT_2, ADC_CHANNEL_3, *voltage);
        }
    }

    return result;
}