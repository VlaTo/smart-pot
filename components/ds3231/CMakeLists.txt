if(${IDF_TARGET} STREQUAL esp32)
    set(REQ "log" "driver" "esp_idf_lib_helpers" "i2cdev")
endif()
set(
    SOURCES
       "ds3231.cpp"
)
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
    REQUIRES ${REQ}
)