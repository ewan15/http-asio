project(http_server)

find_package( Boost REQUIRED COMPONENTS program_options regex )
find_package( spdlog REQUIRED )
find_package( fmt REQUIRED )
find_package(OpenSSL REQUIRED)

include_directories( ${Boost_INCLUDE_DIRS} )
include_directories( ${SPDLOG_DIR} )

set(HEADER_FILES
        include/http_server.h include/tcp_connection.h include/http_struct.h include/logger.h)

add_executable(http_server
        src/main.cpp
        src/http_server.cpp
        src/tcp_connection.cpp
        src/http_struct.cpp
        src/config.cpp
        ${HEADER_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries( http_server OpenSSL::SSL )
target_link_libraries( http_server spdlog )
target_link_libraries( http_server fmt )
target_link_libraries( http_server boost_program_options )

if (ENABLE_INSTALL)
        install(TARGETS http_server DESTINATION bin)
endif()
