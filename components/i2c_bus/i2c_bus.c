#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/i2c.h>
#include "include/i2c_bus.h"

static const char* TAG = "i2c_bus";

static struct i2b_bus_desc {
    i2c_port_t port_num;
    i2c_config_t config;
    SemaphoreHandle_t mutex;   
} i2c_master_bus_desc = {0};

#define SEMAPHORE_TAKE() do { \
        if (!xSemaphoreTake(i2c_master_bus_desc.mutex, pdMS_TO_TICKS(CONFIG_I2CDEV_TIMEOUT))) \
        { \
            ESP_LOGE(TAG, "Could not take port mutex %d", i2c_master_bus_desc.port_num); \
            return ESP_ERR_TIMEOUT; \
        } \
        } while (0)

#define SEMAPHORE_GIVE() do { \
        if (!xSemaphoreGive(i2c_master_bus_desc.mutex)) \
        { \
            ESP_LOGE(TAG, "Could not give port mutex %d", i2c_master_bus_desc.port_num); \
            return ESP_FAIL; \
        } \
        } while (0)

esp_err_t i2c_master_init(const i2c_port_t port_num)
{
    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        .clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    ESP_ERROR_CHECK(i2c_param_config(port_num, &config));
    esp_err_t result = i2c_driver_install(port_num, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    if (ESP_OK == result) {
        memcpy(&i2c_master_bus_desc.config, &config, sizeof(i2c_config_t));
        i2c_master_bus_desc.port_num = port_num;
        i2c_master_bus_desc.mutex = xSemaphoreCreateMutex();
    }
    return result;
}

esp_err_t i2c_device_init(i2c_device_t* dev, const i2c_device_addr_t dev_addr)
{
    if (!dev) {
        return ESP_ERR_INVALID_ARG;
    }

    dev->addr = dev_addr;
    dev->lock = xSemaphoreCreateMutex();

    return ESP_OK;
}

esp_err_t i2c_device_take(const i2c_device_t* dev)
{
    if (!xSemaphoreTake(dev->lock, pdMS_TO_TICKS(CONFIG_I2CDEV_TIMEOUT))) {
        ESP_LOGE(TAG, "Could not take device mutex");
        return ESP_ERR_TIMEOUT;
    }

    return ESP_OK;
}

esp_err_t i2c_device_give(const i2c_device_t* dev)
{
    if (!xSemaphoreGive(dev->lock)) {
        ESP_LOGE(TAG, "Could not take device mutex");
        return ESP_FAIL;
    }

    return ESP_OK;
}

static esp_err_t i2c_master_write_raw(
    const i2c_device_t* dev,
    const void* reg,
    const size_t reg_size,
    const void* data,
    const size_t data_size)
{
    if (!dev || !data) {
        return ESP_ERR_INVALID_ARG;
    }

    SEMAPHORE_TAKE();

    i2c_cmd_handle_t handle = i2c_cmd_link_create();

    i2c_master_start(handle);
    i2c_master_write_byte(handle, (dev->addr << 1) | I2C_MASTER_WRITE, true);
    
    if (reg && reg_size) {
        i2c_master_write(handle, reg, reg_size, true);
    }

    i2c_master_write(handle, data, data_size, true);
    i2c_master_stop(handle);
    
    esp_err_t result = i2c_master_cmd_begin(i2c_master_bus_desc.port_num, handle, pdMS_TO_TICKS(CONFIG_I2CDEV_TIMEOUT));
    
    if (ESP_OK != result) {
        ESP_LOGE(TAG, "I2C write error %d", result);
    }

    i2c_cmd_link_delete(handle);

    SEMAPHORE_GIVE();

    return result;
}

static esp_err_t i2c_master_read_raw(
    const i2c_device_t* dev,
    const void* reg,
    const size_t reg_size,
    void* data,
    const size_t data_size)
{
    if (!dev || !data) {
        return ESP_ERR_INVALID_ARG;
    }

    SEMAPHORE_TAKE();

    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    uint8_t addr_mask = dev->addr << 1;
    
    if (reg && reg_size) {
        i2c_master_start(handle);
        i2c_master_write_byte(handle, addr_mask | I2C_MASTER_WRITE, true);
        i2c_master_write(handle, reg, reg_size, true);
    }

    i2c_master_start(handle);
    i2c_master_write_byte(handle, addr_mask | I2C_MASTER_READ, true);
    i2c_master_read(handle, data, data_size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(handle);
    
    esp_err_t result = i2c_master_cmd_begin(i2c_master_bus_desc.port_num, handle, pdMS_TO_TICKS(CONFIG_I2CDEV_TIMEOUT));

    if (ESP_OK != result) {
        ESP_LOGE(TAG, "I2C read error %d", result);
    }

    i2c_cmd_link_delete(handle);

    SEMAPHORE_GIVE();

    return result;
}

esp_err_t i2c_device_write(const i2c_device_t* dev, const void* data, const size_t data_size)
{
    return i2c_master_write_raw(dev, NULL, 0, data, data_size);
}

esp_err_t i2c_device_read(const i2c_device_t* dev, void* data, const size_t data_size)
{
    return i2c_master_read_raw(dev, NULL, 0, data, data_size);
}

esp_err_t i2c_device_write_reg(const i2c_device_t* dev, uint8_t reg, const void* data, const size_t data_size)
{
    return i2c_master_write_raw(dev, &reg, sizeof(uint8_t), data, data_size);
}

esp_err_t i2c_device_read_reg(const i2c_device_t* dev, uint8_t reg, void* data, const size_t data_size)
{
    return i2c_master_read_raw(dev, &reg, sizeof(uint8_t), data, data_size);
}
