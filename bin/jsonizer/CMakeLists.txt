#---------------------------------------------------------------------------#
# Copyright (c) 2024 Nikita Kaskov <nbering@nil.foundation>
#
# Distributed under the Boost Software License, Version 1.0
# See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt
#---------------------------------------------------------------------------#

cmake_minimum_required(VERSION 3.5)

cm_find_package(CM)
include(CMDeploy)
include(CMSetupVersion)

cm_project(proof2json WORKSPACE_NAME ${CMAKE_WORKSPACE_NAME} LANGUAGES ASM C CXX)

string(TOUPPER ${CURRENT_PROJECT_NAME} UPPER_CURRENT_PROJECT_NAME)
if (CPACK_PACKAGE_VERSION)
    add_compile_definitions(${UPPER_CURRENT_PROJECT_NAME}_VERSION=${CPACK_PACKAGE_VERSION})
endif()

if (NOT Boost_FOUND)
    find_package(Boost COMPONENTS REQUIRED filesystem log log_setup program_options)
endif ()

cm_setup_version(VERSION 0.1.0)

# get header files; only needed by CMake generators,
# e.g., for creating proper Xcode projects
set(${CURRENT_PROJECT_NAME}_HEADERS)

# list cpp files excluding platform-dependent files
list(APPEND ${CURRENT_PROJECT_NAME}_SOURCES
        src/main.cpp
        )

set(${CURRENT_PROJECT_NAME}_HEADERS
        )

add_executable(${CURRENT_PROJECT_NAME}
        ${${CURRENT_PROJECT_NAME}_HEADERS}
        ${${CURRENT_PROJECT_NAME}_SOURCES})

set_target_properties(${CURRENT_PROJECT_NAME} PROPERTIES
        LINKER_LANGUAGE CXX
        EXPORT_NAME ${CURRENT_PROJECT_NAME}
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED TRUE)

list(APPEND libraries
        crypto3::algebra
        crypto3::block
        crypto3::codec
        crypto3::multiprecision
        crypto3::pkpad
        crypto3::pubkey
        crypto3::random
        crypto3::zk

        marshalling::core
        marshalling::crypto3_algebra
        marshalling::crypto3_multiprecision
        marshalling::crypto3_zk

        dbms::plugin
        boost::application

        Boost::filesystem
        Boost::log
)

target_link_libraries(${CURRENT_PROJECT_NAME} ${libraries})

target_include_directories(${CURRENT_PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>

        $<$<BOOL:${Boost_FOUND}>:${Boost_INCLUDE_DIRS}>)

if (APPLE OR NOT ${CMAKE_TARGET_ARCHITECTURE} STREQUAL ${CMAKE_HOST_SYSTEM_PROCESSOR})
    set_target_properties(${CURRENT_PROJECT_NAME} PROPERTIES
            XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${APPLE_CODE_SIGN_IDENTITY}"
            XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM}")
elseif (CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
        set_target_properties(${CURRENT_PROJECT_NAME} PROPERTIES
                COMPILE_FLAGS "-s USE_BOOST_HEADERS=1 -s USE_PTHREADS -s PROXY_TO_PTHREAD"
                LINK_FLAGS "-s USE_BOOST_HEADERS=1 -s USE_PTHREADS -s PROXY_TO_PTHREAD"
                LINK_DIRECTORIES "${CMAKE_BINARY_DIR}/libs/boost/src/boost/stage/lib")
    endif ()

    add_dependencies(${CURRENT_PROJECT_NAME} boost)
endif ()

install(TARGETS ${CURRENT_PROJECT_NAME} RUNTIME DESTINATION bin)
