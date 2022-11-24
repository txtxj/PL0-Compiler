cmake_minimum_required(VERSION 3.22)
project(pl0 C)

set(CMAKE_C_STANDARD 99)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test/ DESTINATION ${EXECUTABLE_OUTPUT_PATH}/)

add_executable(pl0
        src/pl0.c
        src/pl0.h
        src/set.c
        src/set.h)
