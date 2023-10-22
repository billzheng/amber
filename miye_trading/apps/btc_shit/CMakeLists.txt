add_executable(btc_shit main.cpp context.cpp perp_ftx.cpp)
target_link_libraries(btc_shit ws_client util rt pthread ssl crypto ${Boost_LIBRARIES})
