There are two ways to build and run the tests:

The first method builds the version written in main.cpp and runs it.  The commands may vary for your compiler. Use the following commands for clang:

clang++ -std=c++20 src/main.cpp -o main
./main

The second method runs gtest tests from file test/tests.cpp. CMake version 3 or higher must be installed. Use the following commands:

./build.sh [Debug/Release]
cd build/[Debug/Release]/bin
./tests