cmake_minimum_required(VERSION 3.8)
project(C4)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")

set(SOURCE_FILES C4.c)
add_executable(C4 ${SOURCE_FILES})