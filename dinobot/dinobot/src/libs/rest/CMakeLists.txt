add_library(rest_client rest_client.cpp)
target_link_libraries(rest_client OpenSSL::Crypto OpenSSL::SSL)
target_link_libraries(rest_client ${Boost_LIBRARIES})
target_link_libraries(rest_client Threads::Threads)

