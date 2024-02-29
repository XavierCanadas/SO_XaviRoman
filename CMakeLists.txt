cmake_minimum_required(VERSION 3.27)
project(SO_P1 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(SO_P1
        main.c
        crc.c
        crc.h
        fileManager.h
        fileManager.c
        timer.h
        timer.c
)
