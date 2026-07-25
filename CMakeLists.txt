cmake_minimum_required(VERSION 3.5)

project(csvcheck
        VERSION 1.0.0
        LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if (MSVC) 
    add_compile_options(
        /W4
        /WX
    )
else 
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )
endif()

add_executable(csvcheck)

target_sources(csvcheck
    PRIVATE 
        src/csvC_job.c
        src/main.c
)