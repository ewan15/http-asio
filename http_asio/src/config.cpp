#include "config.h"
#include <boost/json.hpp>

#include <boost/property_tree/json_parser.hpp>

Config::Config(std::string& json)
{
    std::stringstream json_ss;
    json_ss << json;
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(json_ss, pt);
    host = pt.get<std::string>("host");
    port = pt.get<int32_t>("port");
    web_path = pt.get<std::string>("web_path");
}