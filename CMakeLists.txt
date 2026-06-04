cmake_minimum_required(VERSION 3.20)
project(trafficlight4ai VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Test)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(tools)
