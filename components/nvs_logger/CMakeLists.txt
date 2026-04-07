idf_component_register(
    SRCS "nvs_logger.cpp"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash
)

target_compile_options(${COMPONENT_TARGET} PRIVATE -std=c++17)
