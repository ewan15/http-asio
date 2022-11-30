//
// Created by ewan on 11/23/22.
//

#include "tcp_connection.h"
#include "http_server.h"
#include "http_struct.h"
#include <iostream>
#include <regex>

void TcpConnection::start() {
  socket_.async_read_some(
          boost::asio::buffer(buffer, DEFAULT_CLIENT_BUFFER_SIZE),
          boost::bind(&TcpConnection::inital_client_accept, shared_from_this(),
                      boost::asio::placeholders::error,
                      boost::asio::placeholders::bytes_transferred));
}

TcpConnection::TcpConnection(boost::asio::io_context &io_context, HttpServer* http_server)
    : socket_(io_context), buffer(DEFAULT_CLIENT_BUFFER_SIZE), http_server(http_server) {}

void TcpConnection::handle_write(const boost::system::error_code & /*error*/,
                                 size_t /*bytes_transferred*/) {}

void TcpConnection::setup_read() {
  socket_.async_read_some(
      boost::asio::buffer(buffer, 1'000'000),
      boost::bind(&TcpConnection::handle_read, shared_from_this(),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}

void TcpConnection::inital_client_accept(
    const boost::system::error_code & /*error*/, // Result of operation.
    std::size_t  bytes_transferred           // Number of bytes copied into the
) {
  // If client sends empty window connection could still be valid.
  if (bytes_transferred == 0) {
    setup_read();
    return;
  }

  using It = std::vector<char>::const_iterator;
  HttpHeaderGrammar<It> httpGrammar;
  HttpRequestHeader httpHeader;

  // Not happy with this...
  auto slash_r = std::remove(buffer.begin(), buffer.end(), '\r');
  buffer.erase(slash_r, buffer.end());

  It iter = buffer.begin(), end = buffer.begin()+bytes_transferred;

  bool canParseHTTPHeaders =
      qi::phrase_parse(iter, end, httpGrammar, qi::ascii::blank, httpHeader);

  if (!canParseHTTPHeaders) {
    // We need to kill the connection
    socket_.close();
  }

  const auto encoded_header_response = build_response(httpHeader);

  boost::asio::async_write(
          socket_, boost::asio::buffer(encoded_header_response),
          boost::bind(&TcpConnection::handle_write, shared_from_this(),
                      boost::asio::placeholders::error,
                      boost::asio::placeholders::bytes_transferred));

  socket_.close();
}

std::string TcpConnection::build_response(HttpRequestHeader httpHeader) {
    const auto status_code = "200";
    const auto status_text = "Ok";
    auto header_response = HttpResponseHeader {
            ._http_version = "1.1",
            .status_code = status_code,
            .status_text = status_text,
    };

    if (httpHeader._http_version != "1.1") {
        header_response.status_code = "400";
        header_response.status_text = "unable to query non 1.1 closing";
        return header_response.encode();
    }

    if (!check_page_safe(httpHeader._uri)) {
        header_response.status_code = "401";
        header_response.status_text = "attempting to access forbidden directory";
        return header_response.encode();
    }

    auto page_contents = http_server->get_page(httpHeader._uri);

    if (!page_contents.has_value()) {
        header_response.status_code = "404";
        header_response.status_text = "can't find! :(";
        return header_response.encode();
    }

    auto encoded_header_response = header_response.encode();
    if (page_contents.has_value()) {
        encoded_header_response += "\n";
        encoded_header_response += *page_contents;
    }
    return encoded_header_response;
}

void TcpConnection::handle_read(
    const boost::system::error_code & /*error*/, // Result of operation.
    std::size_t  /*bytes_transferred*/           // Number of bytes copied into the
) {
}

bool check_page_safe(std::string& path) {
    std::regex self_regex("/(.?\\w)*",
                          std::regex_constants::ECMAScript | std::regex_constants::icase);

    return std::regex_match(path, self_regex);
}