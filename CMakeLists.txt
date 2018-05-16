cmake_minimum_required(VERSION 3.10)
project(Compression C)

set(CMAKE_C_STANDARD 99)

add_executable(Compression main.c image.c bmp.c image.h)