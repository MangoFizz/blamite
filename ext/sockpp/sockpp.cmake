# SPDX-License-Identifier: GPL-3.0-only

set(sockpp-SOURCE_DIR ext/sockpp)

add_library(sockpp STATIC 
    ${sockpp-SOURCE_DIR}/src/acceptor.cpp
    ${sockpp-SOURCE_DIR}/src/connector.cpp
    ${sockpp-SOURCE_DIR}/src/datagram_socket.cpp
    ${sockpp-SOURCE_DIR}/src/exception.cpp
    ${sockpp-SOURCE_DIR}/src/inet_address.cpp
    ${sockpp-SOURCE_DIR}/src/inet6_address.cpp
    ${sockpp-SOURCE_DIR}/src/socket.cpp
    ${sockpp-SOURCE_DIR}/src/stream_socket.cpp
)

set(CMAKE_CXX_FLAGS "-Wcpp")

target_link_libraries(sockpp ${LIBS_SYSTEM})

include_directories(${sockpp-SOURCE_DIR}/include)
