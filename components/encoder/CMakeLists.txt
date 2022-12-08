if(${IDF_TARGET} STREQUAL esp8266)
    set(REQ "esp8266" "freertos" "log" "esp_timer")
elseif(${IDF_VERSION_MAJOR} STREQUAL 4 AND ${IDF_VERSION_MINOR} STREQUAL 1 AND ${IDF_VERSION_PATCH} STREQUAL 3)
    set(REQ "driver" "freertos" "log")
else()
    set(REQ "driver" "freertos" "log" "esp_timer")
endif()
set(
    SOURCES
        "encoder.c"
)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
    REQUIRES ${REQ}
)