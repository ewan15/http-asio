cmake_minimum_required (VERSION 3.17)
set (CMAKE_CXX_STANDARD 20)

project (http-asio)

find_package( Boost REQUIRED COMPONENTS system program_options regex )
include_directories( ${Boost_INCLUDE_DIRS} )
message(${Boost_INCLUDE_DIRS})

if (CLANG_TIDY)
message("running clang tidy")
set(CMAKE_CXX_CLANG_TIDY
        clang-tidy;
        --header-filter=.;
        --config-file=/home/ewan/http-asio/.clang-tidy;
        --checks=performance-*,portability-*,readability-*,bugprone-*;)
endif()

add_subdirectory(http_asio)

