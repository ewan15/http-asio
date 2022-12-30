#include "config.h"
#include "http_server.h"
#include "logger.h"
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <optional>

namespace po = boost::program_options;

std::optional<Config> parse_config(int ac, char** av)
{
    std::string config_path = "/http_asio/config.json";
    po::options_description desc("Allowed options");
    desc.add_options()("help", "produce help message")(
        "path", po::value<std::string>(), "config path");

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return std::nullopt;
    }

    if (vm.count("path")) {
        config_path = vm["path"].as<std::string>();
    }

    std::ifstream config_path_file(config_path.c_str());
    if (!config_path_file.good()) {
        SPDLOG_ERROR("Unable to find config file");
        return std::nullopt;
    }
    config_path_file.seekg(0, std::ios::end);
    std::size_t config_file_size = config_path_file.tellg();
    std::string config_contents(config_file_size, ' ');
    config_path_file.seekg(0);
    config_path_file.read(&config_contents[0], config_file_size);

    const auto config = Config(config_contents);
    return config;
}

int main(int ac, char** av)
{
    spdlog::set_level(spdlog::level::debug);
    SPDLOG_INFO("Running");

    const auto config = parse_config(ac, av);
    if (config == std::nullopt)
        return 1;

    try {
        boost::asio::io_context io_context;

        HttpServer httpServer(io_context, std::move(*config));

        std::vector<std::thread> threads;
        const auto thread_count = std::thread::hardware_concurrency() * 2;
        for (int i = 0; i < thread_count; i++) {
            threads.emplace_back([&] { io_context.run(); });
        }

        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
