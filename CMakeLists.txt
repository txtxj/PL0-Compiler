cmake_minimum_required(VERSION 3.22)
project(pl0 C)

set(CMAKE_C_STANDARD 99)

add_custom_target(copy-files ALL
        COMMAND ${CMAKE_COMMAND} -E
        copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/test" "${CMAKE_CURRENT_BINARY_DIR}/"
        )

add_executable(pl0
        src/pl0.c
        src/pl0.h
        src/set.c
        src/set.h)

add_dependencies(pl0 copy-files)