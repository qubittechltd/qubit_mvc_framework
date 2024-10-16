cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 17)

project(qubit_mvc_framework LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

if(DEFINED Qt6_DIR)
    include(${Qt6_DIR}/qt.toolchain.cmake)
endif()

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Sql
    Network
    Concurrent
)

add_library(qubit_mvc_framework_lib STATIC
    cachefiles.cpp
    formdata.cpp
    middleware.cpp
    qrtdatabase.cpp
    redirect.cpp
    sanitizer.cpp
    secure.cpp
    session.cpp
    utilities.cpp
)

set_target_properties(qubit_mvc_framework_lib PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Sql
    Network
    Concurrent
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(qubit_mvc_framework_lib
    Qt6::Core
    Qt6::Sql
    Qt6::Network
    Qt6::Concurrent
)
