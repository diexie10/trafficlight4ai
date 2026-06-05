cmake_minimum_required(VERSION 3.20)
project(trafficlight4ai VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Multimedia LinguistTools Test)

include(CTest)

add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
add_subdirectory(tools)
