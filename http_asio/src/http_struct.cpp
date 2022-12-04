//
// Created by ewan on 11/26/22.
//
#include <http_struct.h>
#include <string>

std::string HttpResponseHeader::encode()
{
    std::string message;
    message += "HTTP/" + _http_version + " " + status_code + " " + status_text + "\n";

    for (const auto& header : headers) {
        message += header.first + ": " + header.second + "\n";
    }

    return message;
}
