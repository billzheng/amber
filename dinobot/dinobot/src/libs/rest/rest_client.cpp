#include <iostream>
#include "rest_client.h"

#include "../../utils/date.h"

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

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

namespace dinobot { namespace lib { namespace rest {

// Report a failure
void
fail(boost::system::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

//
/// session
//
// Performs an HTTP GET and sends to the file for writing. 
class session : public std::enable_shared_from_this<session>
{
    tcp::resolver resolver_;
    ssl::stream<tcp::socket> stream_;
    boost::beast::flat_buffer buffer_; // (Must persist between reads)
    http::request<http::empty_body> req_;
    http::response<http::string_body> res_;
    std::ofstream log_file_;
    std::string product_;

public:
    // Resolver and stream require an io_context
    explicit
    session(boost::asio::io_context& ioc, ssl::context& ctx)
        : resolver_(ioc)
        , stream_(ioc, ctx)
    {
    }

    ~session()
    {
        log_file_.close();
    }

    // Start the asynchronous operation
    void
    run(
        char const* host,
        char const* port,
        char const* target,
        int version,
        std::string filename,
        std::string product)
    {
        log_file_ = std::ofstream(filename,  std::ios_base::app | std::ios_base::out); 
        product_ = product;

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if(! SSL_set_tlsext_host_name(stream_.native_handle(), host))
        {
            boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            std::cerr << ec.message() << "\n";
            return;
        }

        // Set up an HTTP GET request message
        req_.version(version);
        req_.method(http::verb::get);
        req_.target(target);
        req_.set(http::field::host, host);
        req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Look up the domain name
        resolver_.async_resolve(
            host,
            port,
            std::bind(
                &session::on_resolve,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
    }

    void
    on_resolve(
        boost::system::error_code ec,
        tcp::resolver::results_type results)
    {
        if(ec)
            return fail(ec, "resolve");

        // Make the connection on the IP address we get from a lookup
        boost::asio::async_connect(
            stream_.next_layer(),
            results.begin(),
            results.end(),
            std::bind(
                &session::on_connect,
                shared_from_this(),
                std::placeholders::_1));
    }

    void
    on_connect(boost::system::error_code ec)
    {
        if(ec)
            return fail(ec, "connect");

        // Perform the SSL handshake
        stream_.async_handshake(
            ssl::stream_base::client,
            std::bind(
                &session::on_handshake,
                shared_from_this(),
                std::placeholders::_1));
    }

    void
    on_handshake(boost::system::error_code ec)
    {
        if(ec)
            return fail(ec, "handshake");

        // Send the HTTP request to the remote host
        http::async_write(stream_, req_,
            std::bind(
                &session::on_write,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
    }

    void
    on_write(
        boost::system::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");
        
        // Receive the HTTP response
        http::async_read(stream_, buffer_, res_,
            std::bind(
                &session::on_read,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
    }

    void
    on_read(
        boost::system::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "read");

        // Write the message to standard out
        //std::cout << "XXXXXXXX:"<< res_.body() << ":" << std::endl;
        //log_file_.write(res_.body(), res_.body().size());
        std::chrono::system_clock::time_point received_tp = std::chrono::system_clock::now();
        log_file_   << received_tp.time_since_epoch().count() << " " 
                    << product_ << " "
                    << res_.body() << std::endl;

        // Gracefully close the stream
        stream_.async_shutdown(
            std::bind(
                &session::on_shutdown,
                shared_from_this(),
                std::placeholders::_1));
    }

    void
    on_shutdown(boost::system::error_code ec)
    {
        if(ec == boost::asio::error::eof)
        {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec.assign(0, ec.category());
        }
        if(ec)
        {
            // most cases HTTPS servers close streeam early so we ignore
            if (ec != boost::asio::ssl::error::stream_truncated)
                return fail(ec, "shutdown");
        }

        // If we get here then the connection is closed gracefully
    }
};

/////////////////////////////////////////////////////
rest_client::rest_client(dinobot::lib::configs &c)
    : version_(10)
    //, md_(nullptr)
{
    port_ = static_cast<unsigned int>(c.get_config<int>("exchange_rest", "port"));
    port_ = port_ + 0;
    host_ = c.get_config<std::string>("exchange_rest", "host");

    version_ = 11;

    // save a file for recording the stuff 
    log_file_ = c.get_config<std::string>("logs", "path_rest");
}


rest_client::~rest_client()
{

}

std::string rest_client::send_get_req(std::string& target, const char* product)
{
    std::string temp(product);
    send_get_req(target, temp);

    return std::string("OK");
}

std::string rest_client::send_get_req(std::string& target, std::string &product)
{
    // The io_context is required for all I/O
    boost::asio::io_context ioc;

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::sslv23_client};

    // This holds the root certificate used for verification
    load_root_certificates(ctx);

    // Launch the asynchronous operation
    std::make_shared<session>(ioc, ctx)->run(host_.c_str(), std::to_string(port_).c_str(), target.c_str(), version_, log_file_, product);

    // Run the I/O service. The call will return when
    // the get operation is complete.
    ioc.run();

    return std::string("good");
}

/*
std::string rest_client::send_rest_msg2(std::string resource)
{
    return resource;
}

std::string rest_client::send_rest_msg1(std::string resource)
{
	return resource;
}

std::string rest_client::send_rest_msg(std::string resource)
{
    return resource;
}

std::string rest_client::rec_rest_msg()
{
    return "";
}
*/

}}} // dinobot::lib::rest

