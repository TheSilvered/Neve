include_directories(BEFORE ../main)
include_directories(../../src)

set(Sources
    ../main/nv_test.c
    # nv_context.c is included directly in the tests to test internal functions
    ../../src/nv_mem.c
    ../../src/nv_udb.c
    ../../src/nv_unicode.c
)

file(GLOB TestSources "*.c")

foreach(TestSource ${TestSources})

string(REPLACE ".c" "" TestName ${TestSource})
string(REGEX REPLACE ".*/" "" TestName ${TestName})

add_executable(${TestName} ${TestSource} ${Sources})
add_test(${TestName} ${TestName})

endforeach()
