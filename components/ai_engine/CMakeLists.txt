idf_component_register(
    SRCS "ai_engine.cpp"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash freertos
    PRIV_REQUIRES esp_timer
)

target_compile_options(${COMPONENT_TARGET} PRIVATE -std=c++17)
target_compile_options(${COMPONENT_TARGET} PRIVATE -Wno-misleading-indentation)
target_compile_options(${COMPONENT_TARGET} PRIVATE -Wno-misleading-indentation)
