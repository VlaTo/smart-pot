set(
    SOURCES
        "crc8.cpp"
        "smartpot.cpp"
        "menu_item.cpp"
        "menu.cpp"
        "main.cpp"
)
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "include"
)
