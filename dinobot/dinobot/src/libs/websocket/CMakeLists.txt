add_library(dinobot_websocket websocket.cpp)
target_link_libraries(dinobot_websocket uWS)
target_link_libraries(dinobot_websocket z)
target_link_libraries(dinobot_websocket uv)

