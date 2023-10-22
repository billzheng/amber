add_library(ws_client STATIC ws_client.cpp)
target_include_directories(ws_client PUBLIC)
target_link_libraries(ws_client ws_util)
