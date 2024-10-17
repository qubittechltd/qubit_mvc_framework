if(DEFINED RECIPE_NAME AND RECIPE_NAME STREQUAL qubit_mvc_framework)
    return() #to avoid recursion of including or add project
endif()

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
    app.cpp
)

message(STATUS "The current project name is: ${PROJECT_NAME}")

if(NOT DEFINED VENDOR_BINARY_DIR)
    set(VENDOR_BINARY_DIR "/Users/mac/projects/simple_mvc_sunday/build_Debug/vendor")
endif()
if(NOT DEFINED VENDOR_SRC_DIR)
    set(VENDOR_SRC_DIR "/Users/mac/projects/simple_mvc_sunday/vendor/")
endif()
if(NOT DEFINED VENDOR_PREFIX_DIR)
    set(VENDOR_PREFIX_DIR "/Users/mac/projects/simple_mvc_sunday/build_Debug/vendor")
endif()


# Convert the semicolon-separated string back into a list
string(REPLACE ";" " " VENDOR_RECIPES_AS_STRING "${VENDOR_RECIPES}")
message(STATUS "Vendor recipes: ${VENDOR_RECIPES_AS_STRING}")


add_subdirectory(
  "${VENDOR_SRC_DIR}"
  "${VENDOR_BINARY_DIR}/qubit_mvc_framework/vendor/"
)
# include_interface(/Users/mac/projects/simple_mvc_sunday/vendor/cmake/firebase_cpp_sdk.cmake vendor_repo_libs)
# include_interface(/Users/mac/projects/simple_mvc_sunday/vendor/cmake/qtbase.cmake   )

set_target_properties(qubit_mvc_framework_lib PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(qubit_mvc_framework_lib  vendor_repo_libs)

add_compile_definitions(FIREBASE_PROJECT_NAME="qubit-server-templete")
add_compile_definitions(FIREBASE_PROJECT_NUMBER="000000000")
add_compile_definitions(R_PROJECT_DOMAIN="tz.co.simplemvc")
add_compile_definitions(PROJECT_DOMAIN="127.0.0.1")#simplemvc.co.tz
add_compile_definitions(-DDEBUG_PUBLIC_PATH="${CMAKE_SOURCE_DIR}/public")
