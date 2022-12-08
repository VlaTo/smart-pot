if(${IDF_TARGET} STREQUAL esp32)
    set(REQ "i2cdev" "log" "esp_idf_lib_helpers")
endif()
set(
    SOURCES
        "ds3231.c"
)
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
    REQUIRES ${REQ}
)