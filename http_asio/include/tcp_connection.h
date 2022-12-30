//
// Created by ewan on 11/23/22.
//

#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "http_struct.h"

#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include <boost/circular_buffer.hpp>

#define DEFAULT_CLIENT_BUFFER_SIZE 1'000'000

class HttpServer;

// boost::asio::ip::tcp::socket

template<typename socket_type>
class TcpConnection : public std::enable_shared_from_this<TcpConnection<socket_type>> {
public:
    TcpConnection() = delete;
    TcpConnection(boost::asio::io_context& io_context, HttpServer* http_server);

    socket_type& socket() { return socket_; }
    void start();

private:
    void handle_write(const boost::system::error_code& /*error*/,
        size_t /*bytes_transferred*/);

    void setup_read();
    void handle_read(const boost::system::error_code& /*error*/,
        size_t /*bytes_transferred*/);
    void inital_client_accept(const boost::system::error_code&, size_t);
    std::string build_response(HttpRequestHeader, bool& should_kill_connection);

    HttpServer* http_server;
    socket_type socket_;
    std::string message_;
    std::vector<char> buffer;
};

bool check_page_safe(std::string& path);
template<typename socket_type> socket_type create_socket(boost::asio::io_context& io_context);

#endif // TCP_CONNECTION_H
