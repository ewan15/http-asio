//
// Created by ewan on 11/23/22.
//

#include "http_server.h"
#include "logger.h"
#include "tcp_connection.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <fstream>
#include <iostream>
#include <optional>

HttpServer::HttpServer(boost::asio::io_context& ioContext, Config config)
    : ioContext(ioContext)
    , acceptor_(ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), config.port))
    , config(std::move(config))
{
    acceptNewConnection();
}

void HttpServer::acceptNewConnection()
{
    auto new_connection = std::make_shared<TcpConnection>(ioContext, this);

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&HttpServer::handleAccept, this,
            new_connection,
            boost::asio::placeholders::error));
}

void HttpServer::handleAccept(std::shared_ptr<TcpConnection> new_connection,
    const boost::system::error_code& /*error*/)
{
    SPDLOG_INFO("Client connected");
    new_connection->start();
    HttpServer::acceptNewConnection();
}

std::optional<std::string> HttpServer::get_page(std::string path)
{
    if (path.back() == '/')
        path += "index.html";
    std::ifstream file;
    file.open(config.web_path + path);
    if (!file.is_open())
        return std::nullopt;
    file.seekg(0, std::ios::end);
    size_t file_length = file.tellg();
    std::string buffer(file_length, ' ');
    file.seekg(0);
    file.read(&buffer[0], file_length);
    return buffer;
}
