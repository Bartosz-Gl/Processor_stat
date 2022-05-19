cmake_minimum_required(VERSION 3.21)
project(Processor_stat C)

set(CMAKE_C_FLAGS "-Wall -Wextra -pthread")
set(CMAKE_C_STANDARD 99)

add_executable(Processor_stat main.c Threads.c Threads.h)
