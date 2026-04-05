# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/ectroshop9/esp-idf/components/bootloader/subproject"
  "/home/ectroshop9/smart-hive-master/build/bootloader"
  "/home/ectroshop9/smart-hive-master/build/bootloader-prefix"
  "/home/ectroshop9/smart-hive-master/build/bootloader-prefix/tmp"
  "/home/ectroshop9/smart-hive-master/build/bootloader-prefix/src/bootloader-stamp"
  "/home/ectroshop9/smart-hive-master/build/bootloader-prefix/src"
  "/home/ectroshop9/smart-hive-master/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/ectroshop9/smart-hive-master/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/ectroshop9/smart-hive-master/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
