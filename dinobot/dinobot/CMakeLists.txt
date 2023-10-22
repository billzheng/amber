cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

project("dinobot")

enable_testing()

############################################################################################
## this is only here so i can build and test locally on my laptop in the bus...
## in order to build we need to have installed openssl >= 1.1 & boost > 1.66  
## i used brew to install. 
## in the most recent versions of OSX they replaced openssl with their own crypto lib 
## which breaks all the linked ssl symbols, so that is why we have to use the openssl from
## brew
############################################################################################
#if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#    set(MACOSX TRUE)
#    set(OPENSSL_ROOT_DIR /usr/local/opt/openssl\@1.1/)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/opt/openssl@1.1/include")
#    SET( CMAKE_EXE_LINKER_FLAGS  "-H ${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/opt/openssl@1.1/lib -lcrypto -lssl" )
#endif()

############################################################################################
## TODO README XXX
## set some local user overrides to suit my environment for the time being 
## until there is a proper dev environment etc, my (nathan) build infra is a bit convoluted
## as i have the Mac laptop for bus trips etc, and a fedora box for proper dev at home, that's
## why i have the Darwin if branch as well.
##
## for the time being we will need to set MYHOME here and the checkout location
## since the add_project URL section requires absolute file paths :( 
##
## no matter who we are after this section we need to have set the following variables 
## MYHOME
## SRCTREE
## note: to add trailing slash in these file paths.. 
############################################################################################
#if ($ENV{USER} MATCHES "nathan")
#    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#	set(MYHOME /Users/nathan/)
#    else()
#	set(MYHOME /home/nathan/)
#    endif()
#    set(SRCTREE "${MYHOME}src/crypto/dinobot/")
#    message(STATUS "@@ Buiding as user: " $ENV{USER})
#    message(STATUS "@@ Buiding source tree base: " ${SRCTREE})
##elseif($ENV{USER} MATCHES "autobuild')
##elseif($ENV{USER} MATCHES "')
#endif()

#if (NOT DEFINED MYHOME)
#	message(FATAL_ERROR "Exiting, we did not find the MYHOME variable set, please set and run again" )
#endif()
#if (NOT DEFINED SRCTREE)
#	message(FATAL_ERROR "Exiting, we did not find the SRCTREE variable set, please set and run again" )
#endif()

############################################################################################
## Compliler settings 
############################################################################################
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-Wall -Wextra -Werror ")
endif()

if (NOT CONFIGURED_ONCE)
    set(CMAKE_CXX_FLAGS "${warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    set(CMAKE_C_FLAGS   "${warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#find_package(OpenSSL REQUIRED FLAGS STATIC)
#if (OPENSSL_FOUND)
#    include_directories(${OPENSSL_INCLUDE_DIR})
#endif()
#message(STATUS "@@ OPENSSL LIBS HERE " ${OPENSSL_LIBRARIES})
#message(STATUS "@@ OPENSSL INCLUDE DIRS HERE " ${OPENSSL_INCLUDE_DIR})


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package( Boost REQUIRED COMPONENTS system)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} )

MESSAGE( STATUS "@@ PROJECT_BINARY_DIR: " ${PROJECT_BINARY_DIR} )
MESSAGE( STATUS "@@ CMAKE_CXX_COMPILER_ID_DIR: " ${CMAKE_CXX_COMPILER_ID} )
MESSAGE( STATUS "@@ Boost_USE_STATIC_LIBS: " ${Boost_USE_STATIC_LIBS} )

############################################################################################
## External libraries that live in our source tree. 
##
## 
############################################################################################
include(ExternalProject)
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external )

#ExternalProject_Add(uwebsockets
#    #GIT_REPOSITORY https://github.com/uNetworking/uWebSockets.git
#    #URL "${MYHOME}src/crypto/market_data4/external/uWebSockets.tar.gz"
#    URL "${SRCTREE}external/uWebSockets-20180816.tar.gz"
#    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/uWebSockets/src/uwebsockets/
#    #PREFIX ${CMAKE_CURRENT_BINARY_DIR}/uWebSockets
#    #PREFIX ${EXTERNAL_INSTALL_LOCATION}
#    CONFIGURE_COMMAND ""
#    BUILD_COMMAND make
#    BUILD_IN_SOURCE 1
#    INSTALL_COMMAND make install PREFIX=${CMAKE_CURRENT_BINARY_DIR}/external/uWebSockets
#    #WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/uWebSockets/src/uwebsockets/
#)
include_directories(${EXTERNAL_INSTALL_LOCATION}/uWebSockets/include)
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    link_directories(${EXTERNAL_INSTALL_LOCATION}/uWebSockets/lib)
else()
    link_directories(${EXTERNAL_INSTALL_LOCATION}/uWebSockets/lib64)
endif()


############################################################################################
## This needs to be after everything / no idea why 
############################################################################################
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories( ${Boost_LIBRARY_DIRS})
    add_subdirectory(src)
endif()


############################################################################################
## 
############################################################################################
set(CONFIGURED_ONCE TRUE CACHE INTERNAL
    "A flag showing that CMake has configured at least once.")



############################################################################################
## post build stuff.
## 
## empty for the time being.
## will probably add a section to 'install' the binaries to an install dir for deployment 
## somewhere. though not importatnt yet.
############################################################################################
