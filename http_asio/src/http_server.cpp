//
// Created by ewan on 11/23/22.
//

#include "http_server.h"
#include "logger.h"
#include "tcp_connection.h"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <fstream>
#include <iostream>
#include <openssl/ssl.h>
#include <optional>

using plain_socket = boost::asio::ip::tcp::socket;
using ssl_socket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;

template <>
void HttpServer::acceptNewConnection<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>()
{
    auto new_connection = std::make_shared<TcpConnection<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>>(ioContext, this);

    acceptor_.async_accept(
        new_connection->socket().lowest_layer(),
        [this, new_connection](auto error) {
            const auto ssl_socket = new_connection->socket().native_handle();
            const auto* alpn = reinterpret_cast<const unsigned char*>("hi");
            //                SSL_set_alpn_protos(ssl_socket, alpn, 2);
            const unsigned char* out;
            unsigned int outlen;
            // TODO: this is very bad, need to make async
            new_connection->socket().handshake(
                boost::asio::ssl::stream_base::server);
            const auto ssl = new_connection->socket().native_handle();
            //                SSL_get0_alpn_selected(ssl,&out,&outlen);
            //                SSL_get0_next_proto_negotiated(ssl,&out,&outlen);

            std::cout << "connected" << std::endl;
            std::cout << outlen << std::endl;
            std::cout << std::string(out, out + outlen) << std::endl;

            //                SSL_select_next_proto(&out, &outlen, new_connection->socket().server)

            handleAccept(new_connection, error);
        });
}

template <typename socket_type>
void HttpServer::acceptNewConnection()
{
    auto new_connection = std::make_shared<TcpConnection<socket_type>>(ioContext, this);

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&HttpServer::handleAccept<socket_type>, this,
            new_connection,
            boost::asio::placeholders::error));
}

HttpServer::HttpServer(boost::asio::io_context& ioContext, Config config)
    : ioContext(ioContext)
    , acceptor_(ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), config.port))
    , config(std::move(config))
{
    if (this->config.secure) {
        SPDLOG_INFO("Opening secure connection");
        acceptNewConnection<ssl_socket>();
    } else {
        SPDLOG_INFO("Opening insecure connection");
        acceptNewConnection<plain_socket>();
    }
}

template <typename socket_type>
void HttpServer::handleAccept(std::shared_ptr<TcpConnection<socket_type>> new_connection,
    const boost::system::error_code& /*error*/)
{
    SPDLOG_INFO("Client connected");
    new_connection->start();
    HttpServer::acceptNewConnection<socket_type>();
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
