#cmake -DCMAKE_BUILD_TYPE=Debug

set( CMAKE_CXX_STANDARD 20 )

set( PROJECT_NAME memutil )

project(${PROJECT_NAME} LANGUAGES CXX)

set( CMAKE_ENABLE_EXPORTS true )

if( MSVC )
    # warning level 4 and all warnings as errors
    add_compile_options( /Wall )
    #add_compile_options( /WX ) # error on warning.
    add_compile_options( /MP )
    add_compile_options( "/wd4266" )
    add_compile_options( "/wd4820" )
    add_compile_options( "/wd4710" )
    add_compile_options( "/wd5045" )
else()
    # lots of warnings and all warnings as errors
    add_compile_options( -Wall -Wextra -pedantic -Werror -Wno-unused-parameter )
    # -Wno-sized-deallocation
    add_compile_options( -Wno-unused-variable )
    add_definitions( -Wfatal-errors )
endif()

###################################################################

file( GLOB_RECURSE SOURCES inc/*.hpp src/*.hpp src/*.cpp )

source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} )

add_library( ${PROJECT_NAME_DYNAMIC} SHARED ${SOURCES} )
add_library( ${PROJECT_NAME} STATIC ${SOURCES} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/inc )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )

set( DEPENDENCIES
PUBLIC boost_assert
PRIVATE boost_config
PRIVATE boost_container_hash
PRIVATE boost_core
PRIVATE boost_describe
PRIVATE boost_mp11
PRIVATE boost_predef
PRIVATE boost_static_assert
PRIVATE boost_throw_exception
PRIVATE boost_stacktrace
)

if (MSVC)
    list(APPEND DEPENDENCIES PUBLIC boost_winapi)
    list(APPEND DEPENDENCIES PUBLIC boost_stacktrace_windbg_cached)
else()
    list(APPEND DEPENDENCIES PUBLIC boost_stacktrace_basic)
endif()

message( "DEPENDENCIES: ${DEPENDENCIES}" )

set_property( TARGET ${PROJECT_NAME_DYNAMIC} PROPERTY "LINK_LIBRARY_DEPENDENCIES" true )
set_property( TARGET ${PROJECT_NAME} PROPERTY "LINK_LIBRARY_DEPENDENCIES" true )
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "")
target_compile_definitions(${PROJECT_NAME} PRIVATE MU_STATIC=1 )

if (MSVC)
    list(APPEND LIBS msvcmrtd )
    target_link_options(${PROJECT_NAME} PUBLIC "/INCREMENTAL:NO")
    target_link_libraries( ${PROJECT_NAME} ${DEPENDENCIES} )
else()
    target_link_libraries( ${PROJECT_NAME} ${DEPENDENCIES} stdc++fs )
endif()

if( MSVC )

else()
    if( NOT APPLE )
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    endif()
endif()

set(BOOST_ROOT $ENV{MU_DEPS_PATH}/boost)
message("BOOST_ROOT: ${BOOST_ROOT}")

include_directories( ${BOOST_ROOT}/assert/include )
include_directories( ${BOOST_ROOT}/config/include )
include_directories( ${BOOST_ROOT}/container_hash/include )
include_directories( ${BOOST_ROOT}/core/include )
include_directories( ${BOOST_ROOT}/describe/include )
include_directories( ${BOOST_ROOT}/predef/include )
include_directories( ${BOOST_ROOT}/stacktrace/include )
include_directories( ${BOOST_ROOT}/static_assert/include )
include_directories( ${BOOST_ROOT}/throw_exception/include )
include_directories( ${BOOST_ROOT}/winapi/include )

target_include_directories(${PROJECT_NAME} PUBLIC inc)
target_include_directories(${PROJECT_NAME} PRIVATE src)
