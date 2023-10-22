
add_executable(ftx_rest_sos ftx_rest_sos.cpp)
target_link_libraries(ftx_rest_sos rest util time rt pthread ssl crypto)
