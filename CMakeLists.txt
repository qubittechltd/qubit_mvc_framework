cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 17)

project(qubit_mvc_framework LANGUAGES C CXX)


add_library(qubit_mvc_framework_lib INTERFACE)
target_sources(qubit_mvc_framework_lib
    PRIVATE
        utilities.cpp
)

target_include_directories(qubit_mvc_framework_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

