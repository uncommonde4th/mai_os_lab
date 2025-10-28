cmake_minimum_required(VERSION 3.10)
project(lab2)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

add_executable(array_sum 
    ../main.c
)

target_compile_options(array_sum PRIVATE -Wall -Wextra -Wpedantic)
