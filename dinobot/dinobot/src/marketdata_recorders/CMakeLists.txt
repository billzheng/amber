# market data recorder
add_executable(marketdata_recorder marketdata_recorder.cpp)
target_link_libraries(marketdata_recorder string_time)
target_link_libraries(marketdata_recorder libjsoncpp)
target_link_libraries(marketdata_recorder dinobot_ring_logger)
target_link_libraries(marketdata_recorder ${Boost_LIBRARIES})
target_link_libraries(marketdata_recorder ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(marketdata_recorder OpenSSL::Crypto OpenSSL::SSL)


## list of all the exchanges i have so far implemented
#target_link_libraries(marketdata_recorder libexchange_base)
target_link_libraries(marketdata_recorder binance_exchange)
target_link_libraries(marketdata_recorder binance_websocket)
target_link_libraries(marketdata_recorder coinbase_exchange)
target_link_libraries(marketdata_recorder coinbase_websocket)
target_link_libraries(marketdata_recorder bitmex_exchange)
target_link_libraries(marketdata_recorder bitmex_websocket)
target_link_libraries(marketdata_recorder bitfinex_exchange)
target_link_libraries(marketdata_recorder bitfinex_websocket)
target_link_libraries(marketdata_recorder bitstamp_exchange)
target_link_libraries(marketdata_recorder bitstamp_websocket)
target_link_libraries(marketdata_recorder deribit_exchange)
target_link_libraries(marketdata_recorder deribit_websocket)
target_link_libraries(marketdata_recorder kraken_exchange)
target_link_libraries(marketdata_recorder kraken_websocket)


##
## Only needed for MAC/OSX to get uWS to link correctly as i have no idea why it is not linking properly. 
## this section runs the bash command after binary is linked
## 
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_custom_command(TARGET marketdata_recorder
        POST_BUILD
        COMMAND install_name_tool -change libuWS.dylib ${PROJECT_BINARY_DIR}/external/uWebSockets/lib/libuWS.dylib ${PROJECT_BINARY_DIR}/src/marketdata_recorders/marketdata_recorder
    )
endif()

## TESTING APP BEAST HTTPS
#add_executable(beast_App http_client_sync_ssl.cpp)
#target_link_libraries(beast_App ${Boost_LIBRARIES})
