cmake_minimum_required(VERSION 3.23)
project(parser)

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)

# find_package(fmt REQUIRED)

FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 9.1.0
)
FetchContent_MakeAvailable(fmt)

file(GLOB SOURCE "src/*")

add_executable(${PROJECT_NAME} ${SOURCE})
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
