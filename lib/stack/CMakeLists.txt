cmake_minimum_required(VERSION 3.16)

project(stack C)

add_library(stack SHARED
    src/stack.c
)

target_include_directories(stack
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(stack PROPERTIES
    VERSION 1.0
    SOVERSION 1
)
