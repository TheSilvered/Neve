include_directories(../../src)

add_tests_in_directory(
    # nv_buffer.c is included directly in the tests to test internal functions
    nv_context.c
    nv_file.c
    nv_string.c
    unicode/nv_ucd.c
    unicode/nv_utf.c
)
