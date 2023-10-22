add_library( kraken_websocket  kraken_websocket.cpp)
target_link_libraries( kraken_websocket dinobot_websocket)

add_library( kraken_exchange  kraken_exchange.cpp)
target_link_libraries( kraken_exchange  kraken_websocket)
target_link_libraries( kraken_exchange dinobot_ring_logger)
target_link_libraries( kraken_exchange rest_client)
