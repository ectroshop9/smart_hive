idf_component_register(
    SRCS "ota_update.cpp"
    INCLUDE_DIRS "."
    REQUIRES app_update
)

target_compile_options(${COMPONENT_TARGET} PRIVATE -std=c++17)
