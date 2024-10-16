cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 17)

project(qubit_mvc_framework LANGUAGES C CXX)

add_library(qubit_mvc_framework_lib INTERFACE)

# set_target_properties(qubit_mvc_framework_lib PROPERTIES
#   INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
#                                 "${CMAKE_CURRENT_SOURCE_DIR}/include/model"
# )

set_target_properties(qubit_mvc_framework_lib PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include/model"
)

target_sources(qubit_mvc_framework_lib INTERFACE
    utilities.cpp
    formdata.cpp
    redirect.cpp
)



