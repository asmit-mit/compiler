cmake_minimum_required(VERSION 3.10)
set(CMAKE_BUILD_TYPE Debug)

project(compiler C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(lib/hashmap)

add_executable(compile
    src/main.c

    src/lexer/token.c
    src/lexer/lexer.c

    src/symbol_table/symbol.c
)

target_include_directories(compile
    PRIVATE ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(compile PRIVATE hashmap)
