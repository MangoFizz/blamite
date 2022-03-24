// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include <thread>
#include <blamite/console/command.hpp>

namespace Blamite::Engine {
    std::string ConsoleCommand::name() noexcept {
        return m_name;
    }

    ConsoleCommand::Result ConsoleCommand::execute(std::string args) noexcept {
        auto splitted_args = split_arguments(args);

        if(splitted_args.size() < m_min_args) {
            return COMMAND_RESULT_NOT_ENOUGH_ARGUMENTS;
        }
        if(splitted_args.size() > m_max_args) {
            return COMMAND_RESULT_TOO_MANY_ARGUMENTS;
        }

        if(!m_function(splitted_args)) {
            return COMMAND_RESULT_ERROR;
        }
        return COMMAND_RESULT_SUCCESS;
    }

    ConsoleCommand::ConsoleCommand(std::string name, std::size_t min_args, std::size_t max_args, function_t function) noexcept {
        m_name = name;
        m_min_args = min_args;
        m_max_args = max_args;
        m_function = function;
    }

    std::vector<std::string> ConsoleCommand::split_arguments(std::string args) noexcept {
        std::vector<std::string> args_slices;
        std::string slice;

        bool escaped = false;
        bool in_quotes = false;
        for(char const &c : args) {
            if(escaped) {
                slice.push_back(c);
                escaped = false;
            }
            else {
                if(c == '\\') {
                    escaped = true;
                }
                else if(c == '"') {
                    in_quotes = !in_quotes;
                }
                else if(!in_quotes && c == ' ') {
                    if(!slice.empty()) {
                        args_slices.push_back(slice);
                    }
                }
                else {
                    slice.push_back(c);
                }
            }
        }
        if(!slice.empty()) {
            args_slices.push_back(slice);
        }

        return args_slices;
    }
}
