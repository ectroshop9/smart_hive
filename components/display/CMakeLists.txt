idf_component_register(
    SRCS "display.cpp"
    INCLUDE_DIRS "."
    REQUIRES lvgl esp_lcd esp_lcd_ili9341 driver
)
