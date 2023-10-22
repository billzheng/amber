add_executable(perp_ftx main.cpp context.cpp perp_ftx.cpp)
target_link_libraries(perp_ftx ws_client rest util rt pthread ssl crypto ${Boost_LIBRARIES})
