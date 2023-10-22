
add_executable(binance_md main.cpp)

target_link_libraries(binance_md websockets libjson ssl crypto)