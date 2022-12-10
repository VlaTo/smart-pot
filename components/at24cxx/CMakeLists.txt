if(${IDF_TARGET} STREQUAL esp32)
    set(REQ "log" "driver" "esp_idf_lib_helpers" "i2cdev")
endif()
set(
    SOURCES
       "at24cxx.cpp"
)
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "." "include"
    REQUIRES ${REQ}
)
