cmake_minimum_required(VERSION 3.21)
project(Processor_stat C)

set(CMAKE_C_STANDARD 99)

add_executable(Processor_stat main.c Shared.h Threads.c Threads.h)
