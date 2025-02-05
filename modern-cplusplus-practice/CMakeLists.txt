cmake_minimum_required(VERSION 3.22.1)

project(modern-cplusplus-practice)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message("CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message("CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")

include(FetchContent)
FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 9.1.0
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.12.1
)
set(gtest_build_tests OFF CACHE BOOL "Enable gtest tests" FORCE)

FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.8.0
)
set(benchmark_enable_testing OFF CACHE BOOL "Enable gbench tests" FORCE)

FetchContent_MakeAvailable(googletest googlebenchmark)

# Set compiler flags
add_compile_options(-W -Wall)

# For testing
if(INSTALL_GTEST)
    MESSAGE(STATUS "GTEST ON")

    file(GLOB TEST_SOURCE CONFIGURE_DEPENDS "gtest/*")
    
    add_executable(${PROJECT_NAME}_test ${TEST_SOURCE})
    target_include_directories(${PROJECT_NAME}_test PUBLIC "${PROJECT_SOURCE_DIR}/src")
    
    target_link_libraries(${PROJECT_NAME}_test PRIVATE 
        gtest_main
        fmt::fmt
    )
endif()