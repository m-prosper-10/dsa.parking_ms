cmake_minimum_required(VERSION 4.2)
project(DSA)

set(CMAKE_CXX_STANDARD 20)

add_executable(DSA main.cpp
        models/Car.cpp
        models/Car.h
)
