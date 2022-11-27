//
// Created by ewan on 11/23/22.
//

#include "http_server.h"
#include "tcp_connection.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <fstream>
#include <optional>

HttpServer::HttpServer(boost::asio::io_context& ioContext)
    : ioContext(ioContext),
      acceptor_(ioContext, boost::asio::ip::tcp::endpoint(
                               boost::asio::ip::tcp::v4(), 8081)) {
  acceptNewConnection();
}

void HttpServer::acceptNewConnection() {
  auto new_connection = std::make_shared<TcpConnection>(ioContext, this);

  acceptor_.async_accept(new_connection->socket(),
                         boost::bind(&HttpServer::handleAccept, this,
                                     new_connection,
                                     boost::asio::placeholders::error));
}

void HttpServer::handleAccept(std::shared_ptr<TcpConnection> new_connection,
                              const boost::system::error_code & /*error*/) {
  std::cout << "Client connected!" << std::endl;
  new_connection->start();
  HttpServer::acceptNewConnection();
}

std::optional<std::string> HttpServer::get_page(std::string path) {
    if (path.back() == '/')
        path += "index.html";
    std::ifstream file;
    file.open(PATH_PREFIX + path);
    if (!file.is_open())
        return std::nullopt;
    file.seekg(0, std::ios::end);
    size_t file_length = file.tellg();
    std::cout << path << std::endl;
    std::string buffer(file_length, ' ');
    file.seekg(0);
    file.read(&buffer[0], file_length);
    return buffer;
}
