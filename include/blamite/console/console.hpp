// SPDX-License-Identifier: GPL-3.0-only

#ifndef BLAMITE__CONSOLE__CONSOLE_HPP
#define BLAMITE__CONSOLE__CONSOLE_HPP

#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <utility>
#include <optional>
#include <cpp-terminal/base.hpp>
#include <cpp-terminal/window.hpp>
#include "command.hpp"

namespace Blamite::Engine {
    class Console {
    public:
        using Color = Term::fg;
        using Style = Term::style;

        /**
         * Initialize console
         */
        void init() noexcept;

        /**
         * Read console input
         * NOTE: This does not wait for input
         */
        void read_input() noexcept;

        /**
         * Display a message with color
         */
        void print(Color color, std::string out) noexcept;

        /**
         * Display a message
         */
        void print(std::string out = {}) noexcept;

        /**
         * Display a formatted message with color
         */
        template<typename... Args> void printf(Color color, const char *format, Args... args) noexcept {
            char buff[256];
            std::snprintf(buff, sizeof(buff), format, args...);
            print(color, buff);
        }

        /**
         * Display a formatted message
         */
        template<typename... Args> void printf(const char *format, Args... args) noexcept {
            char buff[256];
            std::snprintf(buff, sizeof(buff), format, args...);
            print(buff);
        }

        /**
         * Print a empty line
         */
        void empty_line() noexcept;

        /**
         * Clear console screen
         */
        void clear() noexcept;

        /**
         * Default constructor
         */
        Console() = default;

        /**
         * Deleted copy constructor
         */
        Console(Console &) = delete;

    private:
        struct ScreenBufferLine {
            std::string text;
            Color color;
        };

        /** Console prompt */
        const std::string c_prompt = "blamite( ";
        
        /** Maximum screen buffer entries */
        const std::size_t c_max_screen_buffer_size = 100;

        /** Maximum commands in history */
        const std::size_t c_max_commands_history_size = 20;

        /** Terminal itself */
        std::unique_ptr<Term::Terminal> m_terminal;

        /** Terminal is attached */
        bool m_term_attached;

        /** Console screen */
        std::unique_ptr<Term::Window> m_screen;

        /** Screen buffer */
        std::deque<ScreenBufferLine> m_screen_buffer;

        /** Input buffer */
        std::string m_input_buffer;

        /** Cursor position */
        std::size_t m_input_cursor_pos;

        /** Commands history */
        std::deque<std::string> m_commands_history;

        /**
         * Copy of history that can be modified by the user. 
         * All changes will be forgotten once a command is submitted.
        */
        std::deque<std::string> m_commands_history_buffer;

        /** Current history entry */
        std::int8_t m_history_pos;

        /** Commands */
        std::vector<std::unique_ptr<ConsoleCommand>> m_commands;

        /**
         * Get console dimentions
         */
        std::pair<int, int> get_size() const noexcept;

        /** 
         * Process input key code
         */
        std::optional<std::string> process_input(int key_code) noexcept;

        /**
         * Render screen buffer
         */
        void render_screen() noexcept;

        /**
         * Execute a command
         */
        void execute_command(std::string command) noexcept;

        /**
         * Register commands
         */
        void register_commands() noexcept;

        /**
         * Split string in equal parts
         */
        static std::vector<std::string> split_line(std::string str, std::size_t slice_size, bool spacing) noexcept;
    };
}

#endif
