@echo off
if not exist build mkdir build
cd build
cmake ..
cmake --build .
ctest -C Debug --output-on-failure
pau