file(GLOB SOURCES "*cc")
add_library(ws_util STATIC ${SOURCES})
target_include_directories(ws_util PUBLIC)
target_link_libraries(ws_util ssl crypto dl z)
