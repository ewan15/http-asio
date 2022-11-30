//
// Created by ewan on 11/23/22.
//

#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "http_struct.h"

#define DEFAULT_CLIENT_BUFFER_SIZE 1'000'000

class HttpServer;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
  TcpConnection() = delete;
  TcpConnection(boost::asio::io_context &io_context, HttpServer* http_server);

  boost::asio::ip::tcp::socket &socket() { return socket_; }
  void start();

private:

  void handle_write(const boost::system::error_code & /*error*/,
                    size_t /*bytes_transferred*/);

  void setup_read();
  void handle_read(const boost::system::error_code & /*error*/,
                   size_t /*bytes_transferred*/);
  void inital_client_accept(const boost::system::error_code &, size_t);
  std::string build_response(HttpRequestHeader);

  HttpServer* http_server;
  boost::asio::ip::tcp::socket socket_;
  std::string message_;
  std::vector<char> buffer;
};

bool check_page_safe(std::string& path);

#endif // TCP_CONNECTION_H
