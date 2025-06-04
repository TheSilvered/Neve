include_directories(BEFORE ../main)

set(Sources ../main/nv_test.c ../../src/nv_str.c)

add_executable(test_strAppend strAppend.c ${Sources})
add_test(strAppend test_strAppend)
add_executable(test_strAppendC strAppendC.c ${Sources})
add_test(strAppendC test_strAppendC)
