cmake_minimum_required(VERSION 3.4)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "Do not build in-source. Please remove CMakeCache.txt and the CMakeFiles/ directory. then build out-of-source.")
endif()

include(rules/miye.cmake)
project(miye_trading)

set(VERSION "1.0.1")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    string(TOLOWER "${CMAKE_BUILD_TYPE}" config)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/dist/${VERSION}/${config}" CACHE PATH 
        "Default install prefix" FORCE)
endif()

miye_enable_build_messages()

#set(BOOST_SOURCE_DIR "/usr/local/boost_1_61_0/include")
#include_directories(${BOOST_SOURCE_DIR})

set(BOOST_ROOT "/home/billzheng/miniconda3/envs/crypto/inlude")

set(PROJECT_ROOT "./")
include_directories(${PROJECT_ROOT} ${PROJECT_ROOT}/tp ${PROJECT_ROOT}/tp/rtl/include)
enable_testing()
include(CTest)

#set(Boost_LIBRARY_DIR "/usr/lib/x86_64-linux-gnu/")
#set(ThirdParty_LIBRARY_DIR "${PROJECT_ROOT}/3rdparty")

include_directories(libs/spdlog/include/ libs/date/include/ libs/cxxopts/include)
#include_directories(${INCLUDE_DIRECTORIES} ${CATCH_INCLUDE_DIR} ${ThirdParty_LIBRARY_DIR}) 
#include_directories(libs/libwebsockets/includes)
#link_directories(libs/libwebsockets/libs)

message(STATUS ${INCLUDE_DIRECTORIES})


set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_USE_MULTITHREADED OFF)
find_package(Boost 1.82 REQUIRED)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(SYSTEM .)
include_directories(libs/ftx)
include_directories(SYSTEM libs)

add_subdirectory(libcore)
add_subdirectory(libs)
add_subdirectory(apps)
#add_subdirectory(trading)
#add_subdirectory(test/trading/alpha/)
#add_subdirectory(3rdparty/date)
