
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

set(INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

include_directories(${INCLUDE_PATH})

add_library(qubit_mvc_framework_lib STATIC

    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/cachefiles.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/common_p.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/controller.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/core_functions.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/extra_defined_fmt.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/formdata.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/google-services.json.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/google_token_auth.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/integrity.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/middleware.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/migrator.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/qfirebaseapp.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/qrtdatabase.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/queues.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/redirect.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/sanitizer.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/secure.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/sequences.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/session.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/system_settings.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/utilities.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/watcher.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/session.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/controller.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/google-services.json.h

    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/model/db_info.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/model/macros.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/model/model.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/model/model_base.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/model/model_base_data.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/model/model_base_impl.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/model/model_base_impl_sql.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/model/model_relationships.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/model/sql_operator.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/model/sql_query.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/model/table_attribute.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/model/table_attribute_base.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/model/table_attribute_impl.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/model/table_base.h
    ${INCLUDE_PATH}/qubit_mvc_framework/utilities/model/table_base_impl.h

    ${INCLUDE_PATH}/qubit_mvc_framework/config/app.h

    ${INCLUDE_PATH}/qubit_mvc_framework/models/job_fail_model.h
    # ${INCLUDE_PATH}/qubit_mvc_framework/models/job_model.h
    ${INCLUDE_PATH}/qubit_mvc_framework/models/migration_history.h
    ${INCLUDE_PATH}/qubit_mvc_framework/models/migration_model.h
    ${INCLUDE_PATH}/qubit_mvc_framework/models/user_model_impl.h


    app.cpp
    cachefiles.cpp
    formdata.cpp
    middleware.cpp
    qrtdatabase.cpp
    queues.cpp
    redirect.cpp
    sanitizer.cpp
    secure.cpp
    session.cpp
    utilities.cpp

    model/model_privates.cpp
)

set_target_properties(qubit_mvc_framework_lib PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_PATH}"
  OUTPUT_NAME qubit_mvc_framework
)

macro(target_compile_definitions_with_default VAR DEFAULT_VALUE)
    if(NOT DEFINED ${VAR})
        target_compile_definitions(qubit_mvc_framework_lib PRIVATE ${VAR}="${DEFAULT_VALUE}")
    else()
        target_compile_definitions(qubit_mvc_framework_lib PRIVATE ${VAR}="${${VAR}}")
    endif()
endmacro()

target_compile_definitions_with_default(FIREBASE_PROJECT_NAME "qubit-server-templete")
target_compile_definitions_with_default(FIREBASE_PROJECT_NUMBER "000000000")
target_compile_definitions_with_default(R_PROJECT_DOMAIN "tz.co.simplemvc")
target_compile_definitions_with_default(PROJECT_DOMAIN "127.0.0.1")
target_compile_definitions_with_default(DEBUG_PUBLIC_PATH "${CMAKE_SOURCE_DIR}/public")

if(DEFINED VENDOR_RECIPES_AS_STRING AND NOT DEFINED VENDOR_RECIPES_AS_LIST)
    string(REPLACE ":" ";" TMP_VENDOR_RECIPES_AS_LIST "${VENDOR_RECIPES_AS_STRING}")
    set(VENDOR_RECIPES_AS_LIST)
    foreach(item IN LISTS TMP_VENDOR_RECIPES_AS_LIST)
        list(APPEND VENDOR_RECIPES_AS_LIST ${item})
    endforeach()
else()
    set(VENDOR_RECIPES_AS_LIST)
    list(APPEND VENDOR_RECIPES_AS_LIST "firebase_cpp_sdk.cmake" "qtbase.cmake" "openssl.cmake" "custom_http_server.cmake")
    list(JOIN VENDOR_RECIPES_AS_LIST ":" VENDOR_RECIPES_AS_STRING)
endif()

message(STATUS "vendor/qubit_mvc_framework/CMakeLists.txt vendor_recipes: ${VENDOR_RECIPES_AS_LIST}")

if(DEFINED RECIPE_NAME AND RECIPE_NAME STREQUAL qubit_mvc_framework)
    return() #to avoid recursion of including or add project
endif()

add_subdirectory("${VENDOR_SRC_DIR}" "${VENDOR_BINARY_DIR}/qubit_mvc_framework/vendor/")

target_link_libraries(qubit_mvc_framework_lib  vendor_repo_libs)

