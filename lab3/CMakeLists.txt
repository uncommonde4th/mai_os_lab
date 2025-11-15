cmake_minimum_required(VERSION 3.10)
project(lab1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(main
        ./parent.c
)

add_executable(reverse_writer
        ./reverse_writer.c
)

target_include_directories(main PRIVATE include)
target_link_libraries(main m)
