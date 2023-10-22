#
macro(miye_enable_build_messages)
    # Contain the names of all targets (libraries and binaries)
    set(TRIDENT_ALL_TARGETS "" CACHE INTERNAL "")
    
    add_custom_target(PrintBuildingMsg ALL COMMAND /bin/true
        COMMENT "Building ${CMAKE_BUILD_TYPE} configuration [${CMAKE_INSTALL_PREFIX}]")

    add_custom_target(PrintBuildFinishedMsg ALL COMMAND /bin/true
        DEPENDS PrintBuildingMsg
        COMMENT "Finished building ${CMAKE_BUILD_TYPE} configuration [${CMAKE_INSTALL_PREFIX}]")
endmacro()

macro(miye_verbose_logging target_name)
    if(DEFINED TRIDENT_ALL_TARGETS)
        set(TRIDENT_ALL_TARGETS ${TRIDENT_ALL_TARGETS} "${target_name}" CACHE INTERNAL "TRIDENT_ALL_TARGETS")
    	add_dependencies(${target_name} PrintBuildingMsg)
    	add_dependencies(PrintBuildFinishedMsg ${target_name})
    endif()
endmacro()

#
# Add an library to the build
#
macro(miye_library libname)
    file(GLOB srcs "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/detail/*.cpp")
    file(GLOB hdrs "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
    file(GLOB private_hdrs "${CMAKE_CURRENT_SOURCE_DIR}/detail/*.hpp")
    if(srcs)
        add_library(${libname} ${srcs})
        set(export_as "PUBLIC")        
    else()
        # Header only library
        add_library(${libname} INTERFACE)
        set(export_as "INTERFACE")
    endif()
    set(header_loc "${CMAKE_CURRENT_SOURCE_DIR}")
    if(CHECK_LIBRARY_DEPS)
        file(RELATIVE_PATH libpath "${CMAKE_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}")
        get_filename_component(libdirname "${libpath}" NAME)
        get_filename_component(libpath "${libpath}" DIRECTORY)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/${libpath}")
        execute_process(COMMAND ${CMAKE_COMMAND}
            -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/include/${libpath}/${libdirname}")
        set(header_loc ${CMAKE_CURRENT_BINARY_DIR}/include)
    endif()
    target_include_directories(${libname} ${export_as}
        "$<BUILD_INTERFACE:${header_loc}>"
        "$<INSTALL_INTERFACE:include/${libname}>")
    set(extra_libs ${ARGN})
    if(extra_libs)
        target_link_libraries(${libname} ${export_as} ${extra_libs})
    endif()
    install(TARGETS ${libname} EXPORT miye
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include/${libname})
    install(FILES ${hdrs} DESTINATION include/${libname})
    miye_verbose_logging(${libname})
endmacro()

#
# Add an application to the build
#

macro(miye_application_single appname src_file)
    #file(GLOB srcs "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
    file(GLOB srcs ${src_file})
    file(GLOB hdrs "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
    add_executable(${appname} ${srcs})
    set(header_loc "")
    target_include_directories(${appname} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    set(extra_libs ${ARGN})
    if (extra_libs)
        target_link_libraries(${appname} PRIVATE ${extra_libs})
    endif()
    install(TARGETS ${appname} EXPORT miye RUNTIME DESTINATION bin)
    miye_verbose_logging(${appname})
endmacro()

macro(miye_application appname)
    file(GLOB srcs "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
    file(GLOB hdrs "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
    add_executable(${appname} ${srcs})
    set(header_loc "")
    target_include_directories(${appname} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    set(extra_libs ${ARGN})
    if (extra_libs)
        target_link_libraries(${appname} PRIVATE ${extra_libs})
    endif()
    install(TARGETS ${appname} EXPORT miye RUNTIME DESTINATION bin)
    miye_verbose_logging(${appname})
endmacro()

#
# Rules
#

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
#set(CXX "/usr/local/gcc-5.2.0/bin/g++")
#set(CC "/usr/local/gcc-5.2.0/bin/gcc")

if(UNIX)
    add_compile_options(-std=c++17 -Wall -O2 -g -Wreturn-type)
endif()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are:  Debug Release." FORCE)
endif()

option(CHECK_LIBRARY_DEPS "Verify inter-library dependencies and header usage" ON)

if(CHECK_LIBRARY_DEPS)
    set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_NO_CYCLES ON)
else()
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

endif()
