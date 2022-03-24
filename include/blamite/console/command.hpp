// SPDX-License-Identifier: GPL-3.0-only

#ifndef BLAMITE__CONSOLE__COMMAND_HPP
#define BLAMITE__CONSOLE__COMMAND_HPP

#include <string>
#include <functional>
#include <cstdint>

namespace Blamite::Engine {
    class ConsoleCommand {
    public:
        using function_t = std::function<bool (std::vector<std::string> &)>;

        enum Result : std::uint8_t {
            COMMAND_RESULT_SUCCESS,
            COMMAND_RESULT_ERROR,
            COMMAND_RESULT_TOO_MANY_ARGUMENTS,
            COMMAND_RESULT_NOT_ENOUGH_ARGUMENTS
        };

        /**
         * Get command name
         */
        std::string name() noexcept;

        /**
         * Execute command function
         */
        Result execute(std::string args) noexcept;

        /**
         * Constructor for command
         * @param name      Name of the commad
         * @param min_args  Minimum arguments for command function
         * @param max_args  Maximum arguments for command function
         * @param function  Function to be executed
         */
        ConsoleCommand(std::string name, std::size_t min_args, std::size_t max_args, function_t function) noexcept;

    private:
        /** Command name */
        std::string m_name;

        /** Minimum arguments */
        std::size_t m_min_args;

        /** Maximum arguments */
        std::size_t m_max_args;

        /** Execute command function in another thread */
        bool run_in_background;

        /** Command function */
        function_t m_function;

        /**
         * Split command arguments
         */
        static std::vector<std::string> split_arguments(std::string args) noexcept;
    };

    namespace Commands {}
}

#endif
