//
// Created by ewan on 11/26/22.
//

#ifndef HTTP_STRUCT_H
#define HTTP_STRUCT_H

#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/qi.hpp>
#include <iostream>
#include <map>
#include <string>

#define HTTP_HEADER_CONNECTION "Connection"
#define HTTP_HEADER_CONNECTION_KEEP_ALIVE "keep-alive"
#define HTTP_HEADER_CONNECTION_CLOSE "close"

#define HTTP_HEADER_CONTENT_LENGTH "Content-Length"

#define HTTP_HEADER_CONTENT_TYPE "Content-Type"

#define HTTP_HEADER_DATE "Date"

#define HTTP_HEADER_SERVER "Server"

const std::string HTTP_404 =
    R"(<html>
<head><title>404 Not Found</title></head>
<body bgcolor="#363636">
<center><h1>404 Not Found</h1></center>
<hr><center>http-asio/beta</center>
</body>
</html>)";

const std::string HTTP_418 =
    R"(<html>
<head><title>418 Teapot</title></head>
<body bgcolor="#363636">
<center><h1>Yum Yum Yum. Teaaa</h1></center>
<hr><center>http-asio</center>
</body>
</html>)";

typedef std::map<std::string, std::string> header_fields_t;

struct HttpRequestHeader {
    std::string _method;
    std::string _uri;
    std::string _http_version;
    header_fields_t _header_fields;
};

struct HttpResponseHeader {
    std::string _http_version;
    std::string status_code;
    std::string status_text;
    std::unordered_map<std::string, std::string> headers;
    std::string encode();
};

BOOST_FUSION_ADAPT_STRUCT(HttpRequestHeader, _method, _uri, _http_version,
    _header_fields)

namespace qi = boost::spirit::qi;

template <typename Iterator, typename Skipper = qi::ascii::blank_type>
struct HttpHeaderGrammar : qi::grammar<Iterator, HttpRequestHeader(), Skipper> {
    HttpHeaderGrammar()
        : HttpHeaderGrammar::base_type(http_header, "HttpHeaderGrammar Grammar")
    {
        method = +qi::alpha;
        uri = +qi::graph;
        http_ver = "HTTP/" >> +qi::char_("0-9.");

        field_key = +qi::char_("0-9a-zA-Z-");
        field_value = +~qi::char_("\n");

        fields = *(field_key >> ':' >> field_value >> qi::lexeme["\n"]);

        http_header = method >> uri >> http_ver >> qi::lexeme["\n"] >> fields;

        BOOST_SPIRIT_DEBUG_NODES((method)(uri)(http_ver)(fields)(http_header))
    }

private:
    qi::rule<Iterator, std::map<std::string, std::string>(), Skipper> fields;
    qi::rule<Iterator, HttpRequestHeader(), Skipper> http_header;
    // lexemes
    qi::rule<Iterator, std::string()> method, uri, http_ver;
    qi::rule<Iterator, std::string()> field_key, field_value;
};

#endif // HTTP_STRUCT_H
