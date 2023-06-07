cmake_minimum_required(VERSION 3.14)

project(flubberpp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CMAKE_BUILD_QT_DEMO "Build Qt demo app" True)

add_subdirectory(lib)
if (CMAKE_BUILD_QT_DEMO)
    add_subdirectory(qtdemo)
endif()
