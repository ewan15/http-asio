//
// Created by ewan on 11/23/22.
//

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "config.h"
#include "tcp_connection.h"
#include <boost/asio.hpp>
#include <optional>

// This is for debug only and will be deleted
#define PATH_PREFIX "/home/ewanbains/html"

class HttpServer {
public:
    HttpServer() = delete;
    explicit HttpServer(boost::asio::io_context& ioContext, Config config);
    std::optional<std::string> get_page(std::string path);

private:
    void handleAccept(std::shared_ptr<TcpConnection>,
        const boost::system::error_code& error);
    void acceptNewConnection();
    Config config;
    boost::asio::io_context& ioContext;
    boost::asio::ip::tcp::acceptor acceptor_;
};

#endif // HTTP_SERVER_H
