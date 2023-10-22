#ifndef _DINOBOT_REST_CLIENT_H
#define _DINOBOT_REST_CLIENT_H

#include <string>

#include "../certs/root_certificates.hpp"
#include "../../libs/configs/configs.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>

namespace dinobot { namespace lib { namespace rest {

class rest_client
{
public: // host , port , ca fie, retry mode, log file

    rest_client(dinobot::lib::configs &);
    //rest_client(std::string, unsigned int, std::string);
    ~rest_client();

    std::string send_get_req(std::string &, std::string &);
    std::string send_get_req(std::string &, const char *);

    /*std::string send_rest_msg(std::string);
    std::string send_rest_msg1(std::string);
    std::string send_rest_msg2(std::string);
    std::string rec_rest_msg();*/ // async response etc

private:
    std::string host_;
    unsigned int port_;
    std::string ca_;
    int version_;
    
    //
    //dinobot::lib::shm::ring_writer *md_;
    std::string log_file_;
};

}}}  // dinobot :: lib :: 

#endif
