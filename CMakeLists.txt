cmake_minimum_required(VERSION 3.21)
project(Processor_stat C)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS "-Wall -Wextra -pthread")
    set(CMAKE_C_STANDARD 99)
    MESSAGE("GNU C++ compiler detected")
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_C_FLAGS "-Weverything -pthread")
    MESSAGE("Clang C++ compiler detected")
endif()

add_executable(Processor_stat main.c Threads.c Threads.h)
