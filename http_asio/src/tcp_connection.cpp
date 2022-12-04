//
// Created by ewan on 11/23/22.
//

#include "tcp_connection.h"
#include "http_server.h"
#include "http_struct.h"
#include "logger.h"
#include <iostream>
#include <regex>
#include <iomanip>
#include <ctime>

void TcpConnection::start()
{
    // Inefficent
    socket_.async_read_some(
        boost::asio::buffer(buffer, DEFAULT_CLIENT_BUFFER_SIZE),
        boost::bind(&TcpConnection::inital_client_accept, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

TcpConnection::TcpConnection(boost::asio::io_context& io_context,
    HttpServer* http_server)
    : socket_(io_context)
    , buffer(DEFAULT_CLIENT_BUFFER_SIZE)
    , http_server(http_server)
{
}

void TcpConnection::handle_write(const boost::system::error_code& /*error*/,
    size_t /*bytes_transferred*/) { }

void TcpConnection::setup_read()
{
    // Inefficent
    socket_.async_read_some(
        boost::asio::buffer(buffer, DEFAULT_CLIENT_BUFFER_SIZE),
        boost::bind(&TcpConnection::inital_client_accept, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void TcpConnection::inital_client_accept(const boost::system::error_code& error,
    std::size_t bytes_transferred)
{
    if (error) {
        SPDLOG_INFO("error on client read. disconnecting");
        socket_.close();
        return;
    }

    // If client sends empty window connection could still be valid.
    if (bytes_transferred == 0) {
        SPDLOG_INFO("client sent empty window attempting to read again");
        setup_read();
        return;
    }

    // Not happy with this...
    auto slash_r = std::remove(buffer.begin(), buffer.end(), '\r');
    buffer.erase(slash_r, buffer.end());

    auto iter = buffer.begin(), end = buffer.begin() + bytes_transferred;
    SPDLOG_INFO("{}", std::string(iter, end));

    HttpHeaderGrammar<decltype(iter)> http_grammer;
    HttpRequestHeader http_header;

    bool canParseHTTPHeaders = qi::phrase_parse(iter, end, http_grammer, qi::ascii::blank, http_header);

    if (!canParseHTTPHeaders) {
        // We need to kill the connection
        SPDLOG_DEBUG("Unable to parse client header, killing");
        socket_.close();
    }

    bool should_kill_connection = false;

    const auto encoded_header_response = build_response(http_header, should_kill_connection);
    SPDLOG_DEBUG("{}", encoded_header_response);

    boost::asio::async_write(
        socket_, boost::asio::buffer(encoded_header_response),
        boost::bind(&TcpConnection::handle_write, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));

    // Checking if keep-alive is enabled
    if (!should_kill_connection && http_header._header_fields.count(HTTP_HEADER_CONNECTION) && http_header._header_fields[HTTP_HEADER_CONNECTION] == HTTP_HEADER_CONNECTION_KEEP_ALIVE) {
        setup_read();
    } else {
        SPDLOG_INFO("closing connection with client due to keep-alive disabled");
        socket_.close();
    }
}

std::string TcpConnection::build_response(HttpRequestHeader httpHeader,
    bool& should_kill_connection)
{
    std::string response_body;

    const auto status_code = "200";
    const auto status_text = "Ok";
    auto header_response = HttpResponseHeader {
        ._http_version = "1.1",
        .status_code = status_code,
        .status_text = status_text,
    };
    header_response.headers[HTTP_HEADER_CONNECTION] = HTTP_HEADER_CONNECTION_KEEP_ALIVE;
    header_response.headers[HTTP_HEADER_SERVER] = "http-asio/beta";

    // Get time as string
    std::time_t t = std::time(nullptr);
    std::string datetime(100,0);
    datetime.resize(std::strftime(&datetime[0], datetime.size(), 
        "%a %d %b %Y - %I:%M:%S%p", std::localtime(&t)));

    header_response.headers[HTTP_HEADER_DATE] = datetime;

    if (httpHeader._http_version != "1.1") {
        SPDLOG_DEBUG("Client [400] sent incompatible HTTP version, killing");
        header_response.status_code = "400";
        header_response.status_text = "unable to query non 1.1 closing";
        should_kill_connection = true;
        return header_response.encode();
    }

    if (!check_page_safe(httpHeader._uri)) {
        SPDLOG_DEBUG("Client [401] attempting to access forbidden directory");
        header_response.status_code = "401";
        header_response.status_text = "attempting to access forbidden directory";
        return header_response.encode();
    }

    if (httpHeader._uri == "/tea" || httpHeader._uri == "/teapot") {
        SPDLOG_DEBUG("Client [418] brewing tea");
        header_response.status_code = "418";
        header_response.status_text = "you found some tea!";
        header_response.headers[HTTP_HEADER_CONTENT_LENGTH] = std::to_string((HTTP_418).size());
        response_body = HTTP_418;
        auto encoded_header_response = header_response.encode();
        encoded_header_response += "\n";
        encoded_header_response += response_body;
        return encoded_header_response;
    }

    auto page_contents = http_server->get_page(httpHeader._uri);

    if (!page_contents.has_value()) {
        SPDLOG_DEBUG("Client [404] attempting to access unknown file");
        header_response.status_code = "404";
        header_response.status_text = "unable to find file";
        header_response.headers[HTTP_HEADER_CONTENT_LENGTH] = std::to_string((HTTP_404).size());
        response_body = HTTP_404;
    }

    if (page_contents.has_value()) {
        header_response.headers[HTTP_HEADER_CONTENT_LENGTH] = std::to_string((*page_contents).size());
        response_body = *page_contents;
    }

    auto encoded_header_response = header_response.encode();
    encoded_header_response += "\n";
    encoded_header_response += response_body;

    SPDLOG_DEBUG("Built 201 response for client");
    return encoded_header_response;
}

void TcpConnection::handle_read(
    const boost::system::error_code& /*error*/, // Result of operation.
    std::size_t /*bytes_transferred*/ // Number of bytes copied into the
)
{
}

bool check_page_safe(std::string& path)
{
    std::regex self_regex("/(.?\\w)*", std::regex_constants::ECMAScript | std::regex_constants::icase);

    return std::regex_match(path, self_regex);
}