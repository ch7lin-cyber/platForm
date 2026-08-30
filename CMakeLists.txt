cmake_minimum_required(VERSION 3.16)

project(system_platform_core LANGUAGES C)

option(PLATFORM_BUILD_TESTS "Build platform unit tests" OFF)

add_subdirectory(L02_HAL)
add_subdirectory(L03_MID_SYSTEM)

if(PLATFORM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()
