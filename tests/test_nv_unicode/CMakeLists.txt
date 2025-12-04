include_directories(BEFORE ../main)

set(Sources
    ../main/nv_test.c
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
