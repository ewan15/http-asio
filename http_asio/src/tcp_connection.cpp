//
// Created by ewan on 11/23/22.
//

#include "tcp_connection.h"
#include "http_server.h"
#include "http_struct.h"
#include <iostream>

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

  if (httpHeader._http_version != "1.1") {
      std::cout << "unable to query non 1.1 closing" << std::endl;
  }

  auto page_contents = http_server->get_page(httpHeader._uri);

  auto header_response = HttpResponseHeader {
      ._http_version = "1.1",
      .status_code = "200",
      .status_text = "Ok",
  };

  auto encoded_header_response = header_response.encode();
  encoded_header_response += "\n";
  encoded_header_response += page_contents;

  boost::asio::async_write(
          socket_, boost::asio::buffer(encoded_header_response),
          boost::bind(&TcpConnection::handle_write, shared_from_this(),
                      boost::asio::placeholders::error,
                      boost::asio::placeholders::bytes_transferred));

  socket_.close();
}

//void TcpConnection::parse_http_request() {}

void TcpConnection::handle_read(
    const boost::system::error_code & /*error*/, // Result of operation.
    std::size_t  /*bytes_transferred*/           // Number of bytes copied into the
) {
}