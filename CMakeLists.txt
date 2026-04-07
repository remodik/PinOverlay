cmake_minimum_required(VERSION 4.2)
project(PinOverlay)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(PinOverlay
        PinOverlay.cpp
        PinOverlay.h)
