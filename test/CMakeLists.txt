include(FetchContent)

FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.6.0
)

FetchContent_MakeAvailable(unity)

add_executable(
    test_csvC_job 
    test_csvC_job.c
)

target_compile_definitions(
    test_csvC_job
    PRIVATE
        TESTDATA_DIR="${CMAKE_SOURCE_DIR}/testdata"
)

target_link_libraries(
    test_csvC_job 
    PRIVATE 
        csvcheck_lib 
        unity
)


add_test(
    NAME 
        csvC_job 
    COMMAND
        test_csvC_job
)