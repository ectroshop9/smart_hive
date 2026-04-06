idf_component_register(
    SRCS "setup_wizard.cpp"
    INCLUDE_DIRS "."
    REQUIRES display ai_engine lvgl freertos
)

target_compile_options(${COMPONENT_TARGET} PRIVATE -std=c++17)
