if(${IDF_TARGET} STREQUAL esp32)
    set(REQ "log" "driver" "esp_adc" "esp_idf_lib_helpers")
endif()
set(
    SOURCES
        "moisture_sensor.cpp"
)
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "include"
    REQUIRES ${REQ}
)