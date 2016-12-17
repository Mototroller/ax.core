cmake_minimum_required (VERSION 2.8.7)
project (AX_CORE_TEST)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic")

set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -march=native")

set(SRC_LIST tests/tests.cpp)
include_directories(${PROJECT_NAME} include)
add_executable(${PROJECT_NAME} ${SRC_LIST})
