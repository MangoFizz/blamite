// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include <vector>
#include <functional>
#include <cpp-terminal/input.hpp>
#include <blamite/console/console.hpp>

namespace Blamite::Engine {
    void Console::init() noexcept {
        const auto [rows, cols] = get_size();
        m_terminal = std::make_unique<Term::Terminal>(true, true, true);
        m_screen = std::make_unique<Term::Window>(cols, rows);
        m_term_attached = Term::Private::is_stdin_a_tty();

        m_input_cursor_pos = 1;
        m_history_pos = 0;

        register_commands();
    }

    void Console::read_input() noexcept {
        try {
            int key;
            if((key = Term::read_key0()) != 0) {
                auto result = process_input(key);
                
                // Re-render prompt after process input
                render_screen();

                if(result.has_value()) {
                    auto command = result.value();
                    if(command == "clear") {
                        this->clear();
                    }
                    else {
                        execute_command(command);
                    }
                }
            }
            else {
                render_screen();
            }
        }
        catch (const std::runtime_error& re) {
            std::cerr << "Runtime error: " << re.what() << std::endl;
        }
        catch (...) {
            std::cerr << "Unknown error." << std::endl;
        }
    }

    void Console::print(Color color, std::string out) noexcept {
        if(m_screen_buffer.size() == c_max_screen_buffer_size) {
            m_screen_buffer.pop_front();
        }
        m_screen_buffer.push_back({out, color});
        render_screen();
    }

    void Console::print(std::string out) noexcept {
        print(Color::white, out);
    }

    void Console::clear() noexcept {
        m_screen_buffer.clear();
        render_screen();
    }

    std::vector<std::string> Console::split_line(std::string str, std::size_t slice_size, bool spacing) noexcept {
        std::vector<std::string> slices;
        std::string slice;
        
        for(char &c : str) {
            if(slice.length() == slice_size) {
                slices.push_back(slice);
                slice.clear();

                if(spacing) {
                    slice = "  "; // Insert spacing before secondary lines
                }
            }
            slice.push_back(c);
        }
        slices.push_back(slice);

        return std::move(slices);
    }

    std::pair<int, int> Console::get_size() const noexcept {
        int rows, cols;
        Term::get_term_size(rows, cols);
        return {rows, cols};
    }

    std::optional<std::string> Console::process_input(int key_code) noexcept {
        std::optional<std::string> result;
        
        // Set up command history buffer
        if(m_commands_history_buffer.empty()) {
            m_commands_history_buffer = m_commands_history;
            m_history_pos = m_commands_history_buffer.size();
            m_commands_history_buffer.push_back(m_input_buffer);
        }
                
        if((key_code >= 'a' && key_code <= 'z') || (key_code >= 'A' && key_code <= 'Z') || (!iscntrl(key_code) && key_code < 128)) {
            m_input_buffer.insert(m_input_buffer.begin() + m_input_cursor_pos - 1, key_code);
            m_input_cursor_pos++;
        }
        else {
            using Key = Term::Key;
            switch(key_code) {
                case Key::BACKSPACE: {
                    if(m_input_cursor_pos > 1) {
                        m_input_buffer.erase(m_input_buffer.begin() + m_input_cursor_pos - 2);
                        m_input_cursor_pos--;
                    }
                    break;
                }
                
                case Key::ARROW_LEFT: {
                    if(m_input_cursor_pos > 1) {
                        m_input_cursor_pos--;
                    }
                    break;
                }

                case Key::ARROW_RIGHT: {
                    if (m_input_cursor_pos <= m_input_buffer.length()) {
                        m_input_cursor_pos++;
                    }
                    break;
                }

                case Key::HOME: {
                    m_input_cursor_pos = 1;
                    break;
                }

                case Key::END: {
                    m_input_cursor_pos = m_input_buffer.length() + 1;
                    break;
                }

                case Key::DEL: {
                    if(m_input_cursor_pos <= m_input_buffer.length()) {
                        m_input_buffer.erase(m_input_buffer.begin() + m_input_cursor_pos - 1);
                    }
                    break;
                }

                case Key::ARROW_UP: {
                    if(m_history_pos > 0) {
                        m_commands_history_buffer[m_history_pos] = m_input_buffer;
                        m_input_buffer = m_commands_history_buffer[--m_history_pos];
                        m_input_cursor_pos = m_input_buffer.length() + 1;
                    }
                    break;
                }

                case Key::ARROW_DOWN: {
                    if(m_history_pos < m_commands_history_buffer.size() - 1) {
                        m_commands_history_buffer[m_history_pos] = m_input_buffer;
                        m_input_buffer = m_commands_history_buffer[++m_history_pos];
                        m_input_cursor_pos = m_input_buffer.length() + 1;
                    }
                    break;
                }

                case Key::ENTER: {
                    if(!m_input_buffer.empty()) {
                        result = m_input_buffer;

                        // Erase current history entry if is
                        if(m_history_pos != m_commands_history_buffer.size() - 1) {
                            m_commands_history.erase(m_commands_history.begin() + m_history_pos);
                        }

                        // Push command to history
                        if(m_commands_history.size() == c_max_commands_history_size) {
                            m_commands_history.pop_front();
                        }
                        m_commands_history.push_back(m_input_buffer);

                        // Clear input
                        m_input_cursor_pos = 1;
                        m_input_buffer.clear();

                        // Clear command history buffer
                        m_commands_history_buffer.clear();
                    }
                    break;
                }

                default: {
                    break;
                }
            }
        }
        return result;
    }

    void Console::render_screen() noexcept {
        const auto [rows, cols] = get_size();

        // Resize buffer if terminal width has changed
        if(m_screen->get_w() != cols) {
            m_screen = std::make_unique<Term::Window>(cols, rows);
        }

        auto max_input_cols = cols - c_prompt.length();
        auto screen_input_lines = split_line(m_input_buffer, max_input_cols, false);
        int screen_cursor_row = (m_input_cursor_pos - 1) / max_input_cols;
        int screen_cursor_col = (m_input_cursor_pos - 1) % max_input_cols + 1;

        // If first input line is full, push a new empty line
        if(screen_input_lines.back().length() == max_input_cols) {
            screen_input_lines.push_back(std::string());
        }

        for(size_t i = 0; i < screen_input_lines.size(); i++) {
            int current_row = rows - i;

            // Print prompt
            std::string prompt;
            if(i == screen_input_lines.size() - 1) {
                prompt = c_prompt;
            }
            else {
                prompt = std::string(c_prompt.length() - 1, '.') + " ";
            }
            m_screen->print_str(1, current_row, prompt);
            m_screen->fill_fg(1, current_row, prompt.length(), current_row, Color::green);
            m_screen->fill_style(1, current_row, prompt.length(), current_row, Style::bold);

            // Print input
            auto line = screen_input_lines.rbegin() + i;
            m_screen->print_str(prompt.length() + 1, current_row, *line + std::string(max_input_cols - line->length(), ' '));
            m_screen->fill_fg(prompt.length() + 1, current_row, cols, current_row, Color::reset);
            m_screen->fill_style(prompt.length() + 1, current_row, cols, current_row, Style::reset);
        }

        std::vector<ScreenBufferLine> screen_buffer_lines;
        std::size_t screen_buffer_max_rows = rows - screen_input_lines.size();
        for(std::size_t i = 0; i < m_screen_buffer.size() && i < screen_buffer_max_rows; i++) {
            auto line = m_screen_buffer.rbegin() + i;
            auto splitted_line = split_line(line->text, cols, true);
            
            auto line_it = splitted_line.rbegin();
            while(line_it != splitted_line.rend()) {
                screen_buffer_lines.insert(screen_buffer_lines.begin(), {*line_it, line->color});
                line_it++;
            }
        }

        auto screen_buffer_lines_bottom = screen_buffer_lines.rbegin();
        for(std::size_t i = 0; i < screen_buffer_max_rows; i++) {
            int current_row = screen_buffer_max_rows - i;
            
            if(screen_buffer_lines_bottom != screen_buffer_lines.rend() && screen_buffer_lines_bottom + i < screen_buffer_lines.rend()) {
                auto line = screen_buffer_lines_bottom + i;
                m_screen->print_str(1, current_row, line->text + std::string(cols - line->text.length(), ' '));
                m_screen->fill_fg(1, current_row, line->text.length(), current_row, line->color);
            }
            else {
                m_screen->print_str(1, current_row, std::string(cols, ' '));
            }
        }

        m_screen->set_cursor_pos(c_prompt.length() + screen_cursor_col, rows - screen_input_lines.size() + screen_cursor_row + 1);

        // Render screen
        std::cout << m_screen->render(1, 1, m_term_attached) << std::flush;
    }

    void Console::execute_command(std::string command) noexcept {
        std::string name = command.substr(0, command.find(' '));
        std::string args;
        if(command.length() > name.length()) {
             args = command.substr(name.size() + 1);
        }
        
        auto it = m_commands.begin();
        while(it != m_commands.end()) {
            auto &command = *it;
            if(command->name() == name) {
                using Result = ConsoleCommand::Result;
                switch(command->execute(args)) {
                    case Result::COMMAND_RESULT_NOT_ENOUGH_ARGUMENTS:
                        printf(Color::gray, "Not enough arguments in \"%s\" command.", name.c_str());
                        break;

                    case Result::COMMAND_RESULT_TOO_MANY_ARGUMENTS: 
                        printf(Color::gray, "Too many arguments in \"%s\" command.", name.c_str());
                        break;

                    default:
                        break;
                }
                break;
            }
            it++;
        }

        if(it == m_commands.end()) {
            printf(Color::gray, "Requested command \"%s\" cannot be executed now.", name.c_str());
        }
    }

    void Console::register_commands() noexcept {
        #define EXTERN_FN(command_name) ({ \
            extern bool command_name(std::vector<std::string> &) noexcept; \
            command_name; \
        })

        #define REGISTER_COMMAND(name, min_args, max_args, function) \
            this->m_commands.emplace_back(std::make_unique<ConsoleCommand>(name, min_args, max_args, EXTERN_FN(function)))

        REGISTER_COMMAND("quit", 0, 0, quit_command);
        REGISTER_COMMAND("ticks", 0, 0, ticks_command);
    }
}
