set(
    SOURCES
        "esp_idf_lib_helpers.c"
)
idf_component_register(
    SOURCES ${SOURCES}
    INCLUDE_DIRS "."
    REQUIRES "freertos"
)