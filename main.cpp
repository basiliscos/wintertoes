#include <iostream>
#include <functional>
#include <memory>
#include <string>

#include <boost/make_unique.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include <beast/http.hpp>
#include <beast/core/streambuf.hpp>

#include "Logging.h"


//using namespace beast;
///using namespace beast::http;

namespace http = beast::http;
namespace asio = boost::asio;
namespace sys = boost::system;

using endpoint_type = boost::asio::ip::tcp::endpoint;
using socket_type = boost::asio::ip::tcp::socket;
using request_type = http::request_v1<http::string_body>;
using response_type = http::response_v1<http::string_body>;

const char* listen_address = "0.0.0.0";
const short port = 7000;

int main () {

    /* initialize logging */
    Apollo::Logging::init();

    /* initialize listening service */
    LOG_INFO("Initializig service at " << listen_address << ":" << port);
    auto io_service = std::make_shared<asio::io_service>();
    endpoint_type endpoint(asio::ip::address::from_string(listen_address), port);


    auto acceptor = std::make_shared<asio::ip::tcp::acceptor>(*io_service);
    acceptor->open(endpoint.protocol());
    acceptor->bind(endpoint);
    acceptor->listen(asio::socket_base::max_connections);

    auto workers_count = boost::thread::physical_concurrency() * 2;
    LOG_INFO("Starting client acceptions at " << endpoint << " using " << workers_count << " workers");

    auto route_request = [io_service](std::unique_ptr<socket_type> peer, std::unique_ptr<request_type> request) mutable {
        spawn(*io_service, [client = std::move(peer), request = std::move(request)] (boost::asio::yield_context yield) mutable {
            sys::error_code error_code;
            http::response_v1<http::string_body> response;
            auto path = request->url;
            response.status = 404;
            response.reason = "Not Found";
            response.version = request->version;
            response.headers.insert("Content-Type", "text/html");
            response.body = "The file '" + path + "' was not found\n";
            http::prepare(response);
            http::async_write(*client, response, yield[error_code]);
            LOG_DEBUG("send response");
        });
    };

    auto read_request = [io_service, route_request](std::unique_ptr<socket_type> peer) mutable {
        spawn(*io_service, [client = std::move(peer), route_request] (boost::asio::yield_context yield) mutable {
            sys::error_code error_code;
            beast::streambuf buffer;
            auto request = boost::make_unique<request_type>();
            http::async_read(*client, buffer, *request, yield[error_code]);
            LOG_DEBUG("got request");
            route_request(std::move(client), std::move(request));
        });
    };

    spawn(*io_service, [acceptor, io_service, read_request] (boost::asio::yield_context yield) mutable {
        while(true) {
            sys::error_code error_code;
            auto peer_socket = boost::make_unique<socket_type>(*io_service);
            acceptor->async_accept(*peer_socket, yield[error_code]);
            if (error_code) {
                LOG_WARN("on accept error: " <<  error_code.message());
                continue;
            }
            LOG_DEBUG("client accepted");
            read_request(std::move(peer_socket));
        }
    });

    boost::thread_group pool;
    for(auto i = 0; i < workers_count; i++) {
        pool.create_thread([io_service] { io_service->run(); });
    }
    pool.join_all();

    return 0;
}
