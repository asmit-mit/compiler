cmake_minimum_required(VERSION 3.10)

project(compiler C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(compile
    src/main.c

    src/lexer/token.c
    src/lexer/lexer.c

    src/hashmap/hashmap.c
    src/hashmap/bucket.c
    src/hashmap/symbol.c
)

target_include_directories(compile
    PRIVATE ${CMAKE_SOURCE_DIR}/include
)
