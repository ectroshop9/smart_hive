cmake_minimum_required(VERSION 3.16)

# أخبر "الشال" أن كل مكوناتك (القديمة والجديدة) موجودة في هذا المجلد
set(EXTRA_COMPONENT_DIRS "components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# اسم المشروع الخاص بك
project(smart_hive_master)