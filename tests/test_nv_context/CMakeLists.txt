include_directories(../../src)

add_tests_in_directory(
    # nv_context.c is included directly in the tests to test internal functions
    nv_string.c
    nv_udb.c
    nv_unicode.c
)
