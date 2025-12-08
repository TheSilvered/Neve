set(CommonSources
    ../main/nv_test.c
    ../../src/nv_mem.c
    ../../src/nv_threads.c # needed for nv_mem.c
)
include_directories(BEFORE main)
add_definitions(-DNV_THREADS_NO_ERROR)

function(add_tests_in_directory)
    file(GLOB TestSources "*.c")
    set(ExpandedSources "")
    foreach(src IN LISTS ARGV)
        list(APPEND ExpandedSources "../../src/${src}")
    endforeach()

    foreach(TestSource ${TestSources})
        string(REPLACE ".c" "" TestName ${TestSource})
        string(REGEX REPLACE ".*/" "" TestName ${TestName})

        add_executable(
            ${TestName}
            ${TestSource}
            ${ExpandedSources}
            ${CommonSources}
        )

        add_test(${TestName} ${TestName})
    endforeach()
endfunction()

add_subdirectory(test_nv_array)
add_subdirectory(test_nv_buffer)
add_subdirectory(test_nv_context)
add_subdirectory(test_nv_string)
add_subdirectory(test_nv_unicode)
