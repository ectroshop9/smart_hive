cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# إضافة المكونات المخصصة
set(EXTRA_COMPONENT_DIRS
    components/display
    components/joystick
    components/espnow_handler
    components/hive_manager
)

project(master_hive)
