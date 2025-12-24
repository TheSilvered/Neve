include_directories(../../src)

add_tests_in_directory(
    nv_buffer
    # nv_buffer.c is included directly in the tests to test internal functions
    nv_context.c
    nv_file.c
    nv_string.c
    nv_mem.c
    nv_threads.c
    unicode/nv_ucd.c
    unicode/nv_utf.c
)
