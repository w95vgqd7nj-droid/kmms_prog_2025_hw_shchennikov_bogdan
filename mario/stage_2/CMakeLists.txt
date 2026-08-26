find_package(Curses REQUIRED)
add_executable(mario_stage_2 main.cpp)
target_include_directories(mario_stage_2 PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(mario_stage_2 PRIVATE ${CURSES_LIBRARIES})