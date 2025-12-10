# @file CMakeLists.txt for UuidV7 module.
#
# © copyright 2025 by Hatem Nabli

cmake_minimum_required(VERSION 3.8)
set(this UuidV7)


set(Headers
            include/UuidV7/UuidV7.hpp)
set(Sources
            src/UuidV7.cpp)

add_library(${this} STATIC ${Headers} ${Sources})

set_target_properties(${this} PROPERTIES
    FOLDER Libraries
)

target_include_directories(${this} PUBLIC include)

add_subdirectory(test)

