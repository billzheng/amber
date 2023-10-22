add_executable(test_arbiter test_arbiter.cpp)
target_link_libraries(test_arbiter time rt pthread ssl crypto)
