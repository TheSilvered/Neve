cmake_minimum_required(VERSION 3.20)

project(Neve VERSION 0.0.1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_C_EXTENSIONS off)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

include_directories(Neve include)

add_executable(Neve
    src/nv_editor.c
    src/nv_main.c
    src/nv_string.c
    src/nv_term.c
    src/nv_unicode.c
)

enable_testing()

add_subdirectory(tests)

add_custom_command(
    TARGET Neve POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/compile_commands.json
)
