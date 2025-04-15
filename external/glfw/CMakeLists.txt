cmake_minimum_required(VERSION 3.10)
project(glfw)

add_library(glfw STATIC
    glfw.c
)

target_include_directories(glfw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
