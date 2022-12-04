#ifndef HTTP_CONFIG_H
#define HTTP_CONFIG_H

#include <string>

struct Config {
    Config() = delete;
    Config(std::string& json);

    std::string host;
    int32_t port;
    std::string web_path;
};

#endif