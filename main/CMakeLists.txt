idf_component_register(SRCS "master_hive.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_lcd esp_wifi nvs_flash esp_timer lvgl)
