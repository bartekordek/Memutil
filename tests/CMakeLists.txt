cmake_minimum_required (VERSION 3.5)

#cmake -DCMAKE_BUILD_TYPE=Debug

get_filename_component( basename ${CMAKE_CURRENT_SOURCE_DIR} NAME )

project( ${basename} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(EXEC ${PROJECT_NAME})

if (MSVC)
    add_compile_options()
    add_compile_options( /MP )
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

include_directories(src)
file( GLOB_RECURSE SOURCES inc/*.hpp src/*.hpp src/*.cpp )
source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} )

set(TEST_EXEC_NAME ${EXEC}_bin)
add_executable(${TEST_EXEC_NAME} ${SOURCES})

target_link_libraries(${TEST_EXEC_NAME} memutil gtest gmock )
target_include_directories(${TEST_EXEC_NAME} PRIVATE inc)
target_include_directories(${TEST_EXEC_NAME} PRIVATE src)

if (MSVC)
    set_target_properties( ${TEST_EXEC_NAME} PROPERTIES LINK_FLAGS "/INCREMENTAL:NO" )

    message( "[MU test] Setting MU tests run dir to: ${CMAKE_CURRENT_SOURCE_DIR}" )
    set_target_properties( ${TEST_EXEC_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
    set_target_properties( ${TEST_EXEC_NAME} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS  "--gtest_break_on_failure" )

    if( ${CMAKE_BUILD_TYPE} MATCHES "Debug" )
        set( GTEST_BIN gtestd )
        set( GMOCK_BIN gmockd )
    else()
        set( GTEST_BIN gtest )
        set( GMOCK_BIN gmock )
    endif()
endif()

set(MU_BINARY_DIR $ENV{MU_ROOT_DIR}/bin)
message("MU_BINARY_DIR: ${MU_BINARY_DIR}")
add_custom_command(TARGET ${TEST_EXEC_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TEST_EXEC_NAME}> ${MU_BINARY_DIR}
)