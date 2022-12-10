if(${IDF_TARGET} STREQUAL esp32)
    set(REQ "log" "driver")
endif()
set(
    SOURCES
        "i2cdev.cpp"
)
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "." "include"
    REQUIRES ${REQ}
)
