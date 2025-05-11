cmake_minimum_required(VERSION 3.20)

project(Neve VERSION 0.0.1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED true)

include_directories(Neve include)

add_executable(Neve src/main.c src/sterm.c)
