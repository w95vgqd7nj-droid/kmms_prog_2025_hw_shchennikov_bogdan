cmake_minimum_required(VERSION 3.14)
project(vector_list)

enable_testing()

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(VECTOR_LIST_LIB "vector_list_lib")
add_library(${VECTOR_LIST_LIB} "")

add_subdirectory(src)
add_subdirectory(tests)