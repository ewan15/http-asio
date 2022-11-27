#include "http_server.h"
#include <boost/asio.hpp>
#include <iostream>

int main() {
  try {
      boost::asio::io_context io_context;
    HttpServer httpServer(io_context);
    io_context.run();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
