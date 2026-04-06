idf_component_register(
    SRCS "web_server.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_http_server json
)

target_compile_options(${COMPONENT_TARGET} PRIVATE -std=c++17)
