cmake_minimum_required(VERSION 3.16)
project(DSA)

set(CMAKE_CXX_STANDARD 20)

add_executable(DSA
        src/main.cpp
        src/ConsoleInput.cpp
        src/ConsoleApp.cpp
        src/ParkingSystem.cpp
)

target_include_directories(DSA PRIVATE include)
