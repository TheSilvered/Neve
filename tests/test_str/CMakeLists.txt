include_directories(BEFORE ../main)

set(Sources ../main/nv_test.c ../../src/nv_str.c)

#[[
add_executable(strNew strNew.c ${Sources})
add_test(strNew strNew)
add_executable(strAppend strAppend.c ${Sources})
add_test(strAppend strAppend)
add_executable(strAppendC strAppendC.c ${Sources})
add_test(strAppendC strAppendC)
]]

file(GLOB TestSources "*.c")

foreach(TestSource ${TestSources})

string(REPLACE ".c" "" TestName ${TestSource})
string(REGEX REPLACE ".*/" "" TestName ${TestName})

add_executable(${TestName} ${TestSource} ${Sources})
add_test(${TestName} ${TestName})

endforeach()
