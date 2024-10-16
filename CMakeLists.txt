cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 17)

project(qubit_mvc_framework LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

function(include_interface dot_cmake_path interface_lib_var)
    set(INTERFACE_LIB ${interface_lib_var})
    include(${dot_cmake_path})
    unset(INTERFACE_LIB)
endfunction()

if(DEFINED Qt6_DIR)
    include(${Qt6_DIR}/qt.toolchain.cmake)
else()

endif()

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

message(STATUS "The current project name is: ${PROJECT_NAME}")

add_subdirectory(
  "/Users/mac/projects/simple_mvc_sunday/vendor/"
  "/Users/mac/projects/simple_mvc_sunday/build_Debug/vendor/qubit_mvc_framework/vendor/"
  # "${CMAKE_CURRENT_BINARY_DIR}/vendor_build"
)
# include_interface(/Users/mac/projects/simple_mvc_sunday/vendor/cmake/firebase_cpp_sdk.cmake vendor_repo_libs)
# include_interface(/Users/mac/projects/simple_mvc_sunday/vendor/cmake/qtbase.cmake   )

set_target_properties(qubit_mvc_framework_lib PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# find_package(Qt6 REQUIRED COMPONENTS
#     Core
#     Sql
#     Network
#     Concurrent
# )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# target_link_libraries(qubit_mvc_framework_lib
#     Qt6::Core
#     Qt6::Sql
#     Qt6::Network
#     Qt6::Concurrent
# )

target_link_libraries(qubit_mvc_framework_lib  vendor_repo_libs)

add_compile_definitions(FIREBASE_PROJECT_NAME="qubit-server-templete")
add_compile_definitions(FIREBASE_PROJECT_NUMBER="000000000")
add_compile_definitions(R_PROJECT_DOMAIN="tz.co.simplemvc")
add_compile_definitions(PROJECT_DOMAIN="127.0.0.1")#simplemvc.co.tz
add_compile_definitions(-DDEBUG_PUBLIC_PATH="${CMAKE_SOURCE_DIR}/public")
