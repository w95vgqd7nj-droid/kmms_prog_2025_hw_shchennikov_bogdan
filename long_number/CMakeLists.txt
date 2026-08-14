cmake_minimum_required(VERSION 3.14)
project(LongNumberProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

add_executable(run_tests test_long_number.cpp long_number.cpp)
target_link_libraries(run_tests gtest_main)

include(GoogleTest)
gtest_discover_tests(run_tests)