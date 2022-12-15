set(
    SOURCES
        "crc8.cpp"
        "smartpot.cpp"
        "main.cpp"
)
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
)
