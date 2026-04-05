idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES 
        utils
        display
        joystick
        espnow_handler
        hive_manager
        alarm
        ai_engine
        system
        esp_psram
    PRIV_REQUIRES 
        lvgl
        nvs_flash
        esp_adc
        esp_lcd
        driver
        freertos
        esp_timer
        esp_event
        esp_netif
        esp_system
)

target_compile_options(${COMPONENT_TARGET} PRIVATE -std=c++17 -Wno-missing-field-initializers)