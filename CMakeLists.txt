cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS
    components/utils
    components/display
    components/joystick
    components/espnow_handler
    components/hive_manager
    components/alarm
    components/ai_engine
    components/system
    components/web_server
    components/setup_wizard
    components/ota_update
)

project(smart_hive_master)
