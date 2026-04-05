idf_component_register(
    SRCS "espnow_handler.cpp"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash esp_wifi esp_event esp_netif
)
