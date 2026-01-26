# @file CMakeLists.txt file for Uuid7Test module
#
# © copyright 2025 by Hatem Nabli

cmake_minimum_required(VERSION 3.20)
set(this UuidV7Test)

set(Sources
            src/UuidV7Tests.cpp)

add_executable(${this} ${Sources})
set_target_properties(${this} PROPERTIES
    FOLDER Tests
)

if(MSVC)
    target_compile_options(${this} PRIVATE /EHsc)
else()
    target_compile_options(${this} PRIVATE -fexceptions)
endif()

target_include_directories(${this} PRIVATE ..)
target_link_libraries( ${this} PUBLIC
    gtest_main
    UuidV7
)

add_test(
    NAME ${this}
    COMMAND ${this}
)
