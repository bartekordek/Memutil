cmake_minimum_required(VERSION 3.0.0)

project(mem_util_main)

set(default_build_type "Debug")
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
        STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

message("CMAKE BUILD TYPE: ${CMAKE_BUILD_TYPE}")

macro(CmakeToWindow InputPath OutputPath)
    string(REGEX REPLACE "/" "\\\\" ${OutputPath} "${InputPath}")
endmacro()

macro(add_sources_group SUBDIRS_LIST)
    foreach(dir ${SUBDIRS_LIST})
        file(GLOB SOURCES ${dir}/*.h ${dir}/*.hpp ${dir}/*.c ${dir}/*.cpp)
        message("Source group ${dir} for ${SOURCES}")
        source_group(${dir} FILES ${SOURCES})
        list(APPEND SOURCE_FILES ${SOURCES})
    endforeach(dir ${SUBDIRS_LIST})
endmacro()

message("CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message("CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
file(RELATIVE_PATH MU_RENEGATE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
message("MU_RENEGATE_DIR: ${MU_RENEGATE_DIR}")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(MU_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(ENV{MU_ROOT_DIR} ${MU_ROOT_DIR})

set(MU_ROOT_DIR ${CMAKE_SOURCE_DIR})
message("MU_ROOT_DIR: ${MU_ROOT_DIR}")
if(MSVC)
    set(MU_OUTPUT_DIR ${MU_ROOT_DIR}/Build-Windows)
else()
    set(MU_OUTPUT_DIR ${MU_ROOT_DIR}/Build-Linux)
endif()

set(ENV{MU_OUTPUT_DIR} ${MU_OUTPUT_DIR})
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MU_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MU_OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MU_OUTPUT_DIR})

set(MU_DEPS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps)
add_subdirectory(${MU_DEPS_PATH}/boost/assert)
add_subdirectory(${MU_DEPS_PATH}/boost/config)
add_subdirectory(${MU_DEPS_PATH}/boost/container_hash)
add_subdirectory(${MU_DEPS_PATH}/boost/core)
add_subdirectory(${MU_DEPS_PATH}/boost/describe)
add_subdirectory(${MU_DEPS_PATH}/boost/mp11)
add_subdirectory(${MU_DEPS_PATH}/boost/predef)
add_subdirectory(${MU_DEPS_PATH}/boost/stacktrace)
add_subdirectory(${MU_DEPS_PATH}/boost/static_assert)
add_subdirectory(${MU_DEPS_PATH}/boost/throw_exception)
add_subdirectory(${MU_DEPS_PATH}/boost/winapi)

set( gtest_force_shared_crt ON CACHE BOOL "" FORCE )
add_subdirectory(${MU_DEPS_PATH}/googletest)

add_subdirectory(Memutil)
add_subdirectory(tests)

set(OPTION_BUILD_TESTS OFF CACHE BOOL "Disable testing" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(OPTION_SELF_CONTAINED ON CACHE BOOL "" FORCE)
set(OPTION_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
