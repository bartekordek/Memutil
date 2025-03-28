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

add_executable(${EXEC} ${SOURCES})

if (MSVC)
    set_target_properties(
        ${EXEC}
        PROPERTIES
            OUTPUT_NAME ${EXEC}
            SUFFIX ".exe"
    )
else()
    set_target_properties(
        ${EXEC}
        PROPERTIES
            OUTPUT_NAME ${EXEC}
            SUFFIX ".bin"
    )
endif()

target_link_libraries(${EXEC} memutil gtest gmock )
target_include_directories(${EXEC} PRIVATE inc)
target_include_directories(${EXEC} PRIVATE src)

if (MSVC)
    set_target_properties( ${EXEC} PROPERTIES LINK_FLAGS "/INCREMENTAL:NO" )

    message( "[MU test] Setting MU tests run dir to: ${CMAKE_CURRENT_SOURCE_DIR}" )
    set_target_properties( ${EXEC} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
    set_target_properties( ${EXEC} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS  "--gtest_break_on_failure" )

    if( ${CMAKE_BUILD_TYPE} MATCHES "Debug" )
        set( GTEST_BIN gtestd )
        set( GMOCK_BIN gmockd )
    else()
        set( GTEST_BIN gtest )
        set( GMOCK_BIN gmock )
    endif()

    set( GTEST_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin/${CMAKE_BUILD_TYPE} )
endif()