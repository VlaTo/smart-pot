if(${IDF_TARGET} STREQUAL esp8266)
    set(REQ "esp8266" "freertos" "esp_idf_lib_helpers")
else()
    set(REQ "driver" "freertos" "esp_idf_lib_helpers")
endif()
set(
    SOURCES
        "i2cdev.c"
)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
    REQUIRES ${REQ}
)
