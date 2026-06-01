cmake_minimum_required(VERSION 3.24)

project(rvc_ooad VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(rvc_core
    src/GridSimulator.cpp
    src/Rvc.cpp
    src/RvcController.cpp
    src/SimulatedHardwareAdapter.cpp
    src/Types.cpp
)

target_include_directories(rvc_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(rvc_core PUBLIC cxx_std_20)

add_executable(rvc_simulator
    src/main.cpp
)

target_link_libraries(rvc_simulator PRIVATE rvc_core)

include(CTest)

if(BUILD_TESTING)
    option(RVC_USE_SYSTEM_GTEST "Use an installed GoogleTest package instead of fetching one" OFF)

    if(RVC_USE_SYSTEM_GTEST)
        find_package(GTest CONFIG QUIET)
    endif()

    if(NOT GTest_FOUND)
        include(FetchContent)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        )
        FetchContent_MakeAvailable(googletest)
    endif()

    add_executable(rvc_tests
        tests/controller_tests.cpp
        tests/rvc_tests.cpp
        tests/system_tests.cpp
        tests/types_tests.cpp
    )

    target_link_libraries(rvc_tests PRIVATE rvc_core GTest::gtest_main)
    target_compile_definitions(rvc_tests PRIVATE RVC_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

    include(GoogleTest)
    gtest_discover_tests(rvc_tests)

    add_test(
        NAME SimulatorCliDefaultRuns
        COMMAND $<TARGET_FILE:rvc_simulator> --ticks 5 --quiet-map
    )

    add_test(
        NAME SimulatorCliContinuousBackwardScenarioRuns
        COMMAND $<TARGET_FILE:rvc_simulator> --scenario ${CMAKE_CURRENT_SOURCE_DIR}/scenarios/continuous_backward.rvc --quiet-map
    )
endif()
