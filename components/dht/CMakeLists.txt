if(${IDF_TARGET} STREQUAL esp32)
    set(REQ "log" "driver" "esp_idf_lib_helpers")
endif()
set(
    SOURCES
       "dht.cpp"
)
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "." "include"
    REQUIRES ${REQ}
)