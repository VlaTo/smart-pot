idf_component_register(
    SRCS "ds3231_device.c"
    INCLUDE_DIRS "include"
    REQUIRES "i2c_bus"
)
