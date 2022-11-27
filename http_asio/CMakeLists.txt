project(http_server)

find_package( Boost REQUIRED COMPONENTS program_options regex )
include_directories( ${Boost_INCLUDE_DIRS} )

set(HEADER_FILES
        include/http_server.h include/tcp_connection.h include/http_struct.h)

add_executable(http_server src/main.cpp src/http_server.cpp src/tcp_connection.cpp src/http_struct.cpp ${HEADER_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE include)
