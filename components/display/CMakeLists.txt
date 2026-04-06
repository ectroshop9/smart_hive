idf_component_register(
    SRCS "display.cpp"
    INCLUDE_DIRS "."
    REQUIRES lvgl esp_lcd driver freertos utils
    PRIV_REQUIRES espressif__esp_lcd_ili9341
)

target_compile_options(${COMPONENT_TARGET} PRIVATE -std=c++17 -Wno-missing-field-initializers -Wno-designator-order)
