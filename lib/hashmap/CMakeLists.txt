cmake_minimum_required(VERSION 3.16)

project(hashmap C)

add_library(hashmap SHARED
    src/hashmap.c
    src/bucket.c
)

target_include_directories(hashmap
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(hashmap PROPERTIES
    VERSION 1.0
    SOVERSION 1
)
