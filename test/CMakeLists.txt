cmake_minimum_required(VERSION 3.24)
project(TestCANbeSerial)

set(CMAKE_CXX_STANDARD 23)

include_directories(.)

add_executable(TestCANbeSerial main.cpp ../source/CANbeSerial.c)

target_include_directories(TestCANbeSerial PRIVATE ..)