set(
    SOURCES
        "u8g2_box.c"
        "u8g2_setup.c"
        "u8g2_buffer.c"
        "u8g2_cleardisplay.c"
        "u8g2_d_memory.c"
        "u8g2_d_setup.c"
        "u8g2_esp32_hal.c"
        "u8g2_font.c"
        "u8g2_fonts.c"
        "u8g2_hvline.c"
        "u8g2_intersection.c"
        "u8g2_ll_hvline.c"
        "u8g2_selection_list.c"
        "u8x8_d_ssd1306_128x64_noname.c"
        "u8x8_display.c"
        "u8x8_gpio.c"
        "u8x8_setup.c"
        "u8x8_8x8.c"
        "u8x8_byte.c"
        "u8x8_cad.c"
        "u8x8_string.c"
        "u8x8_u8toa.c"
        "u8x8_u16toa.c"
)
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "." "include"
    REQUIRES "driver" "freertos"
)