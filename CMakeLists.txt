cmake_minimum_required(VERSION 3.20)

project(neve VERSION 0.0.1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_C_EXTENSIONS off)
set(CMAKE_BUILD_TYPE Debug)

if (${CMAKE_GENERATOR} MATCHES ".*(Make|Ninja).*")
    set(CMAKE_EXPORT_COMPILE_COMMANDS on)
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

include_directories(neve include)

add_executable(neve
    src/nv_buffer.c
    src/nv_context.c
    src/nv_editor.c
    src/nv_error.c
    src/nv_file.c
    src/nv_gapbuffer.c
    src/nv_log.c
    src/nv_main.c
    src/nv_mem.c
    src/nv_screen.c
    src/nv_string.c
    src/nv_term.c
    src/nv_udb.c
    src/nv_unicode.c
)

if(MSVC)
    target_compile_options(neve PRIVATE /W4)
else()
    target_compile_options(neve PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()

add_subdirectory(tests)

# Copy compile_commands.json only with supported generators
if (${CMAKE_GENERATOR} MATCHES ".*(Make|Ninja).*")
    add_custom_command(
        TARGET neve POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/compile_commands.json
    )
endif()
