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
        web_server
        setup_wizard
        nvs_logger
        lvgl
    PRIV_REQUIRES 
        nvs_flash
        esp_adc
        esp_lcd
        driver
        freertos
        esp_timer
        esp_event
        esp_netif
        esp_system
        esp_psram
        esp_pm
)

target_compile_options(${COMPONENT_TARGET} PRIVATE -std=c++17 -Wno-missing-field-initializers)
