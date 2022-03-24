# SPDX-License-Identifier: GPL-3.0-only

set(cpp-terminal_SOURCE_DIR ext/cpp-terminal)

# create and confirgure library target
add_library(cpp-terminal STATIC
    ${cpp-terminal_SOURCE_DIR}/cpp-terminal/base.cpp 
    ${cpp-terminal_SOURCE_DIR}/cpp-terminal/window.cpp 
    ${cpp-terminal_SOURCE_DIR}/cpp-terminal/input.cpp 
    ${cpp-terminal_SOURCE_DIR}/cpp-terminal/private/platform.cpp
)

include_directories(${cpp-terminal_SOURCE_DIR})
