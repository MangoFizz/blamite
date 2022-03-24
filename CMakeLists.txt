# SPDX-License-Identifier: GPL-3.0-only

# Minimum required version of CMake
cmake_minimum_required(VERSION 3.16)

# Set project info
project(blamite
    VERSION 1.0.0
    LANGUAGES CXX
)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Add dependencies
include(dependencies.cmake)

# Add blamite include path
include_directories(include/)

# Blamite core
add_library(blamite-engine STATIC
    src/engine/console/commands/ticks.cpp
    src/engine/console/commands/quit.cpp
    src/engine/console/command.cpp
    src/engine/console/console.cpp
    src/engine/memory/bitstream.cpp
    src/engine/network/packet.cpp
    src/engine/network/server.cpp
    src/engine/engine.cpp
)

# CLI executable
add_executable(blamite-server
    src/server/main.cpp
)

# Set linker flags
set_target_properties(blamite-server PROPERTIES LINK_FLAGS "-static-libgcc -static-libstdc++")

if(WIN32)
	set(PLATFORM_LIBS ws2_32)
endif()

target_link_libraries(blamite-server blamite-engine cpp-terminal sockpp ${PLATFORM_LIBS})
