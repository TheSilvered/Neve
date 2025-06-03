include_directories(BEFORE ../main)

add_executable(test_strAppend strAppend.c ../main/nv_test.c)
add_test(strAppend test_strAppend)
