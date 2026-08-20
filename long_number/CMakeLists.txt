include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(long_number_lib src/long_number.cpp)

target_include_directories(long_number_lib PUBLIC src)


add_executable(run_tests tests/test_long_number.cpp)

target_link_libraries(run_tests PRIVATE long_number_lib gtest_main)

include(GoogleTest)
gtest_discover_tests(run_tests)